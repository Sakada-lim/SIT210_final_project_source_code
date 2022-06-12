from gpiozero import Servo
from picamera import PiCamera
from time import sleep
import glob
import RPi.GPIO
import tkinter as tk
import tkinter.font
RPi.GPIO.setmode(RPi.GPIO.BCM)

# this servo will act as a demonstration to how the locking system will work where we turn it one way to lock and the other way to unlock
servo = Servo(15)

#creating tkinter window for the GUI we are building 
win = tk.Tk()
win.title("Smart fridge UI")
myFont = tkinter.font.Font(family = 'Hekvetica', size = 12 , weight = "bold")

#initialize the camera and rotation it 180 degree base on our condition
camera = PiCamera()
camera.rotation = 180

#check how many we have in the folder so that we can name the next taken picture correctly
stills = glob.glob("/home/pi/limsa/*.jpg")
still_count = len(stills)

# DEFINITION

def LockButtonUI():
    #turn the servo to the max value
    servo.value = 1
def UnlockButtonUI():
    #turn the servo to the min value
    servo.value = -1
    
def CapturePicture():
    # we global the still_count variable so that we can use it in this function
    # each time we call this function we increment still_count by 1 so that we can name the picture correctly
    global still_count
    still_count += 1
    camera.resolution = (2592, 1944)
    print("capturing still photo %s" % still_count)
    camera.capture("/home/pi/limsa/image%s.jpg" % still_count)
def close():
    RPi.GPIO.cleanup()
    win.destroy()

#GUI
LockButton = tk.Button(win, text = "lock the fridge", font = myFont, command = LockButtonUI, bg = "lightgreen", height = 1, width = 24)
LockButton.grid(row = 0, column = 1)
UnlockButton = tk.Button(win, text = "Unlock the fridge", font = myFont, command = UnlockButtonUI, bg = "pink", height = 1, width = 24)
UnlockButton.grid(row = 1, column = 1)
CaptureButton = tk.Button(win, text = "Capture image", font = myFont, command = CapturePicture, bg = "grey", height = 1, width = 24)
CaptureButton.grid(row = 2, column = 1)


win.protocol("WM_DELETE_WINDOW", close)
win.mainloop()