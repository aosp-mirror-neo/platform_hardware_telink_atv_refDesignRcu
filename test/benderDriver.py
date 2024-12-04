#!/usr/bin/python3
"""BenderDriver is a tool for controlling a Bender RCU Raspberry Pi Hat.

It should be run on a Raspberry Pi with a Bender board attached, and can be
adapted to test the integration of the remote control firmware & hardware,
the Bluetooth stack of a TV device, and the TV OS & apps. It provides an
end-to-end test of the remote control that is identical to what the end user
experiences.

Copyright(c) 2024 Google, LLC.
"""

import subprocess
import time

import RPi.GPIO as GPIO


# The reference RCU implements a subset of these buttons. Each pressed
# button connects a column pin to a row pin, and the MCU detects this
# and implements the button press as IR or Bluetooth signaling.
#
# +------+-----------+----------+---------+----------+--------+-------+
# |      |   Col0    |   Col1   |  Col2   |   Col3   |  Col4  | Col5  |
# +======+===========+==========+=========+==========+========+=======+
# | Row0 | POWER     | RIGHT    | VOL_DN  | INFO     | 4      | GREEN |
# | Row1 | INPUT     | DOWN     | CH_DN   | 0        | 3      | RED   |
# | Row2 | BOOKMARK  | BACK     | YOUTUBE | SUBTITLE | 2      | 6     |
# | Row3 | ASSIST    | HOME     | NETFLIX | 9        | 1      | 5     |
# | Row4 | DASHBOARD | GUIDE    | APP03   | 8        | BLUE   |       |
# | Row5 | UP        | VOL_UP   | APP04   | 7        | YELLOW |       |
# | Row6 | LEFT      | CH_UP    | FREETV  |          |        |       |
# | Row7 | CENTER    | VOL_MUTE |         |          |        |       |
# +------+-----------+----------+---------+----------+--------+-------+

# Map from a button name to the MCU row and column
# Which buttons work depends on the RCU firmware
buttons = {
    "0": (1, 3),
    "1": (3, 4),
    "2": (2, 4),
    "3": (1, 4),
    "4": (0, 4),
    "5": (3, 5),
    "6": (2, 5),
    "7": (5, 3),
    "8": (4, 3),
    "9": (3, 3),
    "APP03": (4, 2),
    "APP04": (5, 2),
    "ASSIST": (3, 0),
    "BACK": (2, 1),
    "BLUE": (4, 4),
    "BOOKMARK": (2, 0),
    "CENTER": (7, 0),
    "CH_DN": (1, 2),
    "CH_UP": (6, 1),
    "DASHBOARD": (4, 0),  # The settings gear button
    "DOWN": (1, 1),
    "FREETV": (6, 2),
    "GREEN": (0, 5),
    "GUIDE": (4, 1),
    "HOME": (3, 1),
    "INFO": (0, 3),
    "INPUT": (1, 0),
    "LEFT": (6, 0),
    "NETFLIX": (3, 2),
    "POWER": (0, 0),
    "RED": (1, 5),
    "RIGHT": (0, 1),
    "SUBTITLE": (2, 3),
    "UP": (5, 0),
    "VOL_DN": (0, 2),
    "VOL_MUTE": (7, 1),
    "VOL_UP": (5, 1),
    "YELLOW": (5, 4),
    "YOUTUBE": (2, 2),
}


# The Bender HW allows us to connect the rows and columns of the keyboard
# matrix by configuring various ICs though Raspberry Pi GPIOs:
#
#     +--------+
#     |  U104  |
#     |        |
# Col0|S1-\    |
# Col2|S2--+>D1+-----------------------+
# Col3|S3--/   |                       |
#     |S4-/    |                       |
#     +--------+                       |
#                                      |
#     +--------+                       |
#     |  U105  |                       |
#     |        |                       |
# Col1|S1-\    |                       |
# Col4|S2--+>D1+-----+    +---------+  |
# Col5|S3--/   |     |    |  U106   |  |
#     |S4-/    |     |    |         |  |
#     +--------+     +----+COM1--NO1+--+
#                    |    |         |  |
#     +--------+     |    +---------+  |
#     |  U102  |     |                 |
#     |        |     |                 |
# Row0|S1-\    |     |                 |
# Row2|S2--+>D1+-----+                 |
# Row4|S3--/   |                       |
# Row5|S4-/    |                       |
#     +--------+                       |
#                                      |
#     +--------+                       |
#     |  U103  |                       |
#     |        |                       |
# Row1|S1-\    |                       |
# Row3|S2--+>D1+-----------------------+
# Row6|S3--/   |
# Row7|S4-/    |
#     +--------+

# rows[r] gives the IC and switch port for a particular MCU row
rows = [
    ("U102", 1),  # Row0
    ("U103", 1),  # Row1
    ("U102", 2),  # Row2
    ("U103", 2),  # Row3
    ("U102", 3),  # Row4
    ("U102", 4),  # Row5
    ("U103", 3),  # Row6
    ("U103", 4),  # Row7
]

# cols[c] gives the IC and switch port for a particular MCU col
cols = [
    ("U104", 1),  # Col0
    ("U105", 1),  # Col1
    ("U104", 2),  # Col2
    ("U104", 3),  # Col3
    ("U105", 2),  # Col4
    ("U105", 3),  # Col5
    ("U105", 4),  # Col6 (actually not connected)
    ("U105", 4),  # Col7 (actually not connected)
]

# These ICs have a separate enable pin mapped to a GPIO. Other ICs are always
# enabled.
icsWithEnable = ["U102", "U103"]

# U106 has ports COM1 and NO1. If our button press combines ICs across these
# ports, we need to enable U106
icsOnCOM1 = ["U102", "U105"]
icsOnNO1 = ["U103", "U104"]

# The GPIO pinout of the Bender HW
pins = {
    # The operator/test status output RGB LED, an output
    "RED_LED_OUT": 21,
    "GREEN_LED_OUT": 19,
    "BLUE_LED_OUT": 23,

    # The red/green status LED set by the remote control firmware (input)
    # Used to confirm keypresses were sent, pairing mode entered, etc.
    "RED_LED": 24,
    "GREEN_LED": 26,

    # Backlight LED and FindMyRemote input/output
    # Used to test the FindMyRemote & backlight features, or can be used as an
    # output to the operator.
    "BACKLIGHT": 22,
    "BUZZER": 37,

    # Detects the audio jack insertion (input)
    # Audio jack is used to send audio from RaspberryPi to the Assistant
    "JACK_DETECT": 18,

    # CPU reset for the Remote control MCU. Low = reset, high = operating
    "RESET": 11,

    # Switch ICs to bridge the remote control keyboard matrix.
    # Used to press buttons.
    "U102A0": 31,
    "U102A1": 32,
    "U102EN": 29,
    "U103A0": 36,
    "U103A1": 12,
    "U103EN": 33,
    "U104A0": 35,
    "U104A1": 38,
    "U105A0": 15,
    "U105A1": 16,
    "U106IN2": 7,
}


def pin_set(p, v):
  """Sets a named GPIO pin.

  Args:
    p: The name of the pin
    v: Boolean value True/high or False/low
  """
  if v:
    # print("  Setting " + p + " HIGH")
    GPIO.output(pins[p], GPIO.HIGH)
  else:
    # print("  Setting " + p + " LOW")
    GPIO.output(pins[p], GPIO.LOW)


def switch_set(ic, s):
  s = s - 1
  pin_set(ic + "A0", s & 1)
  pin_set(ic + "A1", s & 2)


def release_switches():
  """Releases all held switches."""
  print("\tReleasing switches")
  pin_set("U106IN2", 0)
  pin_set("U102EN", 0)
  pin_set("U103EN", 0)
  switch_set("U105", 4)
  switch_set("U104", 4)

  # We expect that the green status LED turns off after we release the buttons
  wait_for_led(green_seq=[0])
  pin_set("BLUE_LED_OUT", 0)
  pin_set("GREEN_LED_OUT", 0)
  pin_set("RED_LED_OUT", 0)


def press_buttons(btns: list[str], down_time: float = 0.1, release=True):
  """Presses one or more named buttons on the remote control.

  Args:
    btns: List of button name strings.
    down_time: How long to press the buttons.
    release: If True, releases the buttons at the end; otherwise, leaves them
      held.
  """
  release_time = time.monotonic() + down_time
  touched_ics = set()

  # Setup the switch inputs for the one or more buttons
  for b in btns:
    r, c = buttons[b]
    ric, rsw = rows[r]
    cic, csw = cols[c]
    touched_ics.add(ric)
    touched_ics.add(cic)
    switch_set(ric, rsw)
    switch_set(cic, csw)

    # Determine if U106 needs to be activated to bridge com1/no1
    if (ric in icsOnCOM1 and cic in icsOnNO1) or (
        cic in icsOnCOM1 and ric in icsOnNO1
    ):
      pin_set("U106IN2", 1)

  # Enable the ICs that were touched and have an enable
  for eic in icsWithEnable:
    if eic in touched_ics:
      pin_set(eic + "EN", 1)

  if not wait_for_led(timeout=down_time):
    pin_set("RED_LED_OUT", 1)
    release_time += 0.1
    print("Warning: RF button press failed. Holding longer for IR")
  else:
    pin_set("GREEN_LED_OUT", 1)
  while time.monotonic() < release_time:
    time.sleep(0.01)
  if release:
    release_switches()


def led_flash(color=2, times=1):
  """Flash the operator status RGB LED.

  Args:
    color: 0 -> 4 color sequence; otherwise, interpreted as r/g/b for bits 0/1/2
    times: How many times to flash
  """
  delay = 0.1
  while times > 0:
    times -= 1
    if color % 8 == 0:
      # Flash the Google colors
      for r, g, b in [(0, 0, 1), (0, 1, 0), (1, 1, 0), (1, 0, 0)]:
        pin_set("BLUE_LED_OUT", b)
        pin_set("GREEN_LED_OUT", g)
        pin_set("RED_LED_OUT", r)
        time.sleep(2 * delay)
    else:
      pin_set("RED_LED_OUT", color & 1)
      pin_set("GREEN_LED_OUT", color & 2)
      pin_set("BLUE_LED_OUT", color & 4)
      time.sleep(delay)

    pin_set("BLUE_LED_OUT", 0)
    pin_set("GREEN_LED_OUT", 0)
    pin_set("RED_LED_OUT", 0)
    time.sleep(delay)

  time.sleep(0.5)


def read_leds():
  return (
      1 if GPIO.input(pins["GREEN_LED"]) == GPIO.HIGH else 0,
      1 if GPIO.input(pins["RED_LED"]) == GPIO.HIGH else 0,
  )


def monitor_led(timeout=1.0):
  """Polls for changes in the red/green status LEDs.

  Args:
    timeout: maximum time to wait for a change.
  """
  now = time.monotonic()
  start = now
  timeout_time = now + timeout
  polling_interval = 0.001
  last_leds = None
  last_leds_time = now
  while now < timeout_time:
    time.sleep(polling_interval)
    now = time.monotonic()
    cur_leds = read_leds()
    if cur_leds != last_leds:
      last_leds = cur_leds
      print("\t\t%f (%f): %s" % (now - start, now - last_leds_time, cur_leds))
      last_leds_time = now


def wait_for_led(
    green_seq: list[int] = None,
    red_seq: list[int] = None,
    timeout=5,
    min_duration=0.001,
):
  """Tests a sequence of red/green LEDs output by the remote control firmware.

  This allows detecting error states of the firmware, completion of commands
  like button presses, etc.

  Args:
    green_seq: The sequence of green LEDs to be observed
    red_seq: The sequence of red LEDs to be observed
    timeout: Maximum time to wait for the LED sequence
    min_duration: minimum time for each stage of the LED sequence

  Returns:
    True if passed, false otherwise.
  """
  if green_seq is None:
    green_seq = [1]
  if red_seq is None:
    red_seq = [0] * len(green_seq)

  expectations = tuple(zip(green_seq, red_seq))

  now = time.monotonic()
  start = now
  timeout_time = now + timeout
  polling_interval = 0.001

  wait_print = True
  first_exp = expectations[0]
  while now < timeout_time:
    time.sleep(polling_interval)
    now = time.monotonic()
    cur_leds = read_leds()

    if cur_leds == first_exp:
      print("\tFirst LEDs expectation achieved")
      break
    # We want an LED to be on, so wait while all LEDs are off
    elif first_exp[0] or first_exp[1]:
      if cur_leds == (0, 0):
        if wait_print:
          print("\tWaiting for any LED to turn on...")
          wait_print = False
        continue
      else:  # Wrong LED(s) are on
        print("\tWrong LEDs on")
        return False
    # We want all LEDs off, so wait while any LED is on
    else:
      if wait_print:
        print("\tWaiting for all LEDs to turn off...")
        wait_print = False
      continue

  step = 0
  num_steps = len(expectations)
  step_end_time = now + min_duration
  print(
      "\tStarting step %d of LED sequence: %s, %fs remaining total time"
      % (step, expectations[step], timeout_time - now)
  )
  while now < timeout_time:
    time.sleep(polling_interval)
    now = time.monotonic()
    cur_leds = read_leds()

    if cur_leds == expectations[step]:
      # Need to monitor that the LEDs meet expectations for the min_duration
      if now < step_end_time:
        # print("... " + str(step_end_time - now) + " " + str(cur_leds))
        continue
      elif step == num_steps - 1:
        print("waitForLED: Success")
        return True
    else:
      # If the LEDs don't match, but we have already matched for min_duration
      # try the next step in the sequence
      print(
          "\t\tStep %d Current LEDs:%s, Expected LEDs:%s, Time elapsed:%fs,"
          " Step time remaining:%fs"
          % (
              step,
              cur_leds,
              expectations[step],
              now - start,
              step_end_time - now,
          )
      )
      if now >= step_end_time and step < num_steps - 1:
        step += 1
        step_end_time = now + min_duration
        print(
            "\tStarting step %d of LED sequence: %s, %fs remaining total time"
            % (step, expectations[step], timeout_time - now)
        )
        continue
      else:
        print(
            "\tAt step %d of LED sequence: wanted %s, got %s"
            " with %fs remaining step time"
            % (step, expectations[step], cur_leds, step_end_time - now)
        )
        monitor_led(timeout=timeout_time - now)
        return False
    # print("---")

  print("\tAt step %d of LED sequence: timed out" % step)
  return False


def assistant(text):
  espeak_cmd = 'espeak -a 200 "%s"' % text

  press_buttons(["ASSIST"], release=False)
  time.sleep(10)
  print(espeak_cmd)
  completed_proc = subprocess.run(
      espeak_cmd, shell=True, check=True, timeout=30
  )
  print(completed_proc.stdout if completed_proc.stdout else "")
  print(completed_proc.stderr if completed_proc.stderr else "")
  release_switches()


def test_backlight():
  """Tests the backlight functionality, both input and output."""
  GPIO.setup(pins["BACKLIGHT"], GPIO.OUT)
  print(
      "Flashing backlight. If you can see this flashing, this test won't work"
  )
  for _ in range(10):
    time.sleep(0.1)
    GPIO.output(pins["BACKLIGHT"], GPIO.HIGH)
    time.sleep(0.1)
    GPIO.output(pins["BACKLIGHT"], GPIO.LOW)

  GPIO.setup(pins["BACKLIGHT"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
  now = time.monotonic()
  timeout_time = now + 10
  passed = False

  led_flash(3)
  print("Checking that backlight is OFF...")
  if (
      GPIO.input(pins["BACKLIGHT"]) == GPIO.HIGH
      or GPIO.wait_for_edge(pins["BACKLIGHT"], GPIO.BOTH, timeout=2000)
      is not None
  ):
    print("Backlight is ON")
    led_flash(1, 3)
    return

  # Press a button and see if the backlight responds
  press_buttons(["DOWN"])
  while time.monotonic() < timeout_time:
    print("Waiting for backlight...")
    channel = GPIO.wait_for_edge(pins["BACKLIGHT"], GPIO.BOTH, timeout=2000)
    if channel is not None:
      print("Detected backlight.")
      led_flash(2, 3)
      passed = True
      break
  if not passed:
    print("Failed to detect backlight.")
    print("Detection only works with the BL_on switch in the OFF position")
    led_flash(1, 3)


def test_find_my_remote():
  """Tests the buzzer functionality, input and output.

  User needs to activate "find my remote" from the TV before the timeout.
  """
  GPIO.setup(pins["BUZZER"], GPIO.OUT)
  print("Sounding buzzer. If you can hear this sound, this test won't work")
  for _ in range(200):
    time.sleep(0.001)
    GPIO.output(pins["BUZZER"], GPIO.HIGH)
    time.sleep(0.001)
    GPIO.output(pins["BUZZER"], GPIO.LOW)

  GPIO.setup(pins["BUZZER"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
  now = time.monotonic()
  timeout_time = now + 10
  passed = False
  led_flash(3)
  while time.monotonic() < timeout_time:
    print("Waiting for buzzer...")
    channel = GPIO.wait_for_edge(pins["BUZZER"], GPIO.BOTH, timeout=2000)
    if channel is not None:
      print("Detected sound on buzzer. Checking LED")
      # Expect a slow blinking RED LED
      if wait_for_led(
          green_seq=[0] * 5,
          red_seq=[1, 0, 1, 0, 1],
          min_duration=0.4,
          timeout=10,
      ):
        passed = True
        led_flash(2, 3)
      break
  if not passed:
    print("Failed to detect sound on buzzer or LEDs.")
    print("Detection only works with the FMR_on switch in the OFF position")
    led_flash(1, 3)

  reset()  # Silence the buzzer


def setup():
  """Sets up the Raspberry Pi GPIOs to run the Bender board."""
  print("Setup...")
  GPIO.setwarnings(False)
  GPIO.setmode(GPIO.BOARD)

  # Set all pins to OUTPUT and LOW
  for p in range(41):
    try:
      GPIO.setup(p, GPIO.OUT)
    except:
      pass

    try:
      pin_set(p, 0)
    except:
      pass

  GPIO.setup(pins["JACK_DETECT"], GPIO.IN, pull_up_down=GPIO.PUD_UP)
  GPIO.setup(pins["GREEN_LED"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
  GPIO.setup(pins["RED_LED"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
  GPIO.setup(pins["BUZZER"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
  GPIO.setup(pins["BACKLIGHT"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

  print("BenderDriver for v0.3 hardware")
  pin_set("RESET", 1)

  if GPIO.input(pins["JACK_DETECT"]) == GPIO.LOW:
    print("Warning: Audio jack out; Assistant can't receive audio")
  else:
    print("Audio jack detected")


def reset():
  print("Resetting controller...")
  pin_set("RESET", 0)
  led_flash(0, 1)
  pin_set("RESET", 1)
  # Sometimes the red LED briefly lights
  GPIO.wait_for_edge(pins["RED_LED"], GPIO.FALLING, timeout=500)
  wait_for_led(green_seq=[1, 0, 1, 0, 1])


# ------------------------------------------------------------------------------

setup()
cmd, lastcmd, l = "", "", 0
while True:
  cmd = input("> ")
  if not cmd:
    cmd = lastcmd
    print(cmd)
  if not cmd:
    continue
  cmd = cmd.upper()
  lastcmd = cmd
  if cmd == "PAIR":
    press_buttons(("BACK", "HOME"), 5.0)
  elif cmd == "CLEAR":  # Clear bond cache
    press_buttons(("CENTER", "VOL_MUTE"), 5.0)
  elif cmd == "BUG":  # Bugreport
    press_buttons(("CENTER", "BACK"), 1.5)
  elif cmd == "TALKBACK":  # Accessibility shortcut
    press_buttons(("BACK", "DOWN"), 1.5)
  elif cmd == "EXIT":
    break
  elif cmd == "LED":
    led_flash(l, max(1, l / 8))
    l += 1
  elif cmd == "RESET":
    reset()
  elif cmd == "FINDMY":
    test_find_my_remote()
  elif cmd == "BACKLIGHT":
    test_backlight()
  else:
    multipleButtons = cmd.split(" ")
    if multipleButtons[0] == "ASSIST" and len(multipleButtons) > 1:
      assistant(" ".join(multipleButtons[1:]))
    else:
      press_buttons(multipleButtons)
