import time
import threading
import board
import digitalio
import RPi.GPIO as GPIO
import adafruit_ssd1306
from PIL import Image, ImageDraw, ImageFont
from w1thermsensor import W1ThermSensor, SensorNotReadyError, Unit, Sensor

width = 128
height = 64

def get_font_size(text, font):
    left, top, right, bottom = font.getbbox(text)
    return right-left, bottom-top

def display_title(draw, font, row_start, row_end, title):
    global width
    global height
    wheight = row_end - row_start + 1
    draw.rectangle((0, row_start, width, row_end), outline=0, fill=0)  # Clear display
    set_temp_text = title
    set_temp_width, set_temp_height = get_font_size(set_temp_text, font=font)
    set_temp_x = (width - set_temp_width) // 2
    set_temp_y = row_start + (wheight // 4) - (set_temp_height // 2)
    draw.text((set_temp_x, set_temp_y), set_temp_text, font=font, fill=1)


def display_temperature(draw, font, row_start, row_end, curr_temp, target_temp):
    global width
    global height
    wheight = row_end - row_start + 1
    draw.rectangle((0, row_start, width, row_end), outline=0, fill=0)  # Clear display
    set_temp_text = 'Curr Temp: {:.1f}F'.format(curr_temp)
    set_temp_width, set_temp_height = get_font_size(set_temp_text, font=font)
    set_temp_x = (width - set_temp_width) // 2
    set_temp_y = row_start + (wheight // 4) - (set_temp_height // 2)
    draw.text((set_temp_x, set_temp_y), set_temp_text, font=font, fill=1)

    if target_temp is not None:
        target_temp_text = 'Trgt Temp: {:.1f}F'.format(target_temp)
        target_temp_width, target_temp_height = get_font_size(target_temp_text, font=font)
        target_temp_x = (width - target_temp_width) // 2
        target_temp_y = row_start + (3 * wheight // 4) - (target_temp_height // 2)
        draw.text((target_temp_x, target_temp_y), target_temp_text, font=font, fill=1)

def read_temperature_with_retry(sensor, retries=5, delay=1):
    for _ in range(retries):
        try:
            return sensor.get_temperature(Unit.DEGREES_F)
        except SensorNotReadyError:
            time.sleep(delay)
    raise SensorNotReadyError("Failed to read temperature after multiple retries")

def main():
    global width
    global height

    print("Testing LED control")
    test_led = 16  # Physical Pin 13

    # GPIO setup
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(test_led, GPIO.OUT, initial=GPIO.LOW)

    # Temperature sensor setup
    sensor = W1ThermSensor(sensor_type=Sensor.DS18B20)

    # Setup OLED display (I2C) for 128x32
    width  = 128
    height = 64
    i2c = board.I2C()
    # disp = adafruit_ssd1306.SSD1306_I2C(width, height, i2c, addr=0x31)
    disp = adafruit_ssd1306.SSD1306_I2C(width, height, i2c, addr=0x3C)
    # disp = adafruit_ssd1306.SSD1306_I2C(width, height, i2c)
    disp.fill(1)
    disp.show()
    time.sleep(5)

    image = Image.new('1', (width, height))
    draw = ImageDraw.Draw(image)

    draw.rectangle((0, 0, disp.width, disp.height), outline=255, fill=255)
    border = 5
    draw.rectangle((border, border, disp.width-border-1, disp.height-border-1),
            outline = 0, fill = 0)
    print(f"Width = {disp.width}, Height = {disp.height}")


    # Load a larger font
    font_path = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
    font_size = 12
    font = ImageFont.truetype(font_path, font_size)
    display_title(draw, font, 0, 16, "Yogurt Maker")
    for i in range(10):
        temperature = read_temperature_with_retry(sensor)
        time.sleep(2)
        GPIO.output(test_led, GPIO.HIGH)
        display_temperature(draw, font, 16, height, temperature, 140.0)
        disp.image(image)
        disp.show()
        time.sleep(2)
        GPIO.output(test_led, GPIO.LOW)

    disp.fill(0)
    disp.show()
    GPIO.cleanup()
    print("Program done.")

if __name__ == "__main__":
    main()
