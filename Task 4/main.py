import time
import cv2 as cv
import numpy as np
import queue
from queue import Queue
import threading

CAMERA_NAME="/dev/video0"
class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")

class SensorX(Sensor):
    '''Sensor X'''
    def __init__(self,delay:float):
        self._delay=delay
        self._data=0
    
    def get(self) -> int:
        time.sleep(self._delay)
        self._data+=1
        return self._data

class SensorCam(Sensor):
    def __init__(self,cam_id):
        self._capture=cv.VideoCapture(cam_id)
    def get(self):
        return self._capture.read()
    def __delete__(self, instance):
        self._capture.release()
        return
class WindowImage():
    def __init__(self,delay) -> None:
        self._delay=delay
    def show(self,img):
        time.sleep(self._delay)
        cv.imshow("output",img)
        return
    def __delete__(self, instance):
        cv.destroyWindow("output")
        return
    
stop_threads=False
def sensor_getter(sensor:Sensor, res_queue:Queue):
    global stop_threads
    while not stop_threads:
        res_queue.put(sensor.get())
        
def draw_sensor_readings(image, r1, r2, r3,
                         font=cv.FONT_HERSHEY_SIMPLEX,
                         scale=0.6, thickness=1,
                         color=(0, 255, 0), margin=10,
                         line_gap=5):
    """
    Draws three integer sensor readings on the bottom-right corner of an image.

    Args:
        image:      numpy array (OpenCV image)
        r1, r2, r3: integer sensor readings
        font:       OpenCV font type
        scale:      font scale
        thickness:  text thickness
        color:      text color (BGR)
        margin:     distance from the right/bottom edge
        line_gap:   vertical spacing between lines
    """
    h, w = image.shape[:2]
    texts = [f"S1: {r1}", f"S2: {r2}", f"S3: {r3}"]
    y_base = h - margin

    # Draw from bottom (S3) to top (S1)
    for i, text in enumerate(reversed(texts)):
        (tw, th), _ = cv.getTextSize(text, font, scale, thickness)
        x = w - margin - tw
        y = y_base - i * (th + line_gap)
        cv.putText(image, text, (x, y), font, scale, color, thickness)


window = WindowImage(1)
sensor0, sq0 = SensorX(0.01), Queue(1000)
sensor1, sq1 = SensorX(0.1), Queue(1000)
sensor2, sq2 = SensorX(1), Queue(1000)
camera, sqc=SensorCam(CAMERA_NAME),Queue(10)
s0_getter_thread=threading.Thread(target=sensor_getter,args=(sensor0,sq0))
s1_getter_thread=threading.Thread(target=sensor_getter,args=(sensor1,sq1))
s2_getter_thread=threading.Thread(target=sensor_getter,args=(sensor2,sq2))
camera_getter_thread=threading.Thread(target=sensor_getter,args=(camera,sqc))

s0_current,s1_current,s2_current,camera_current=0,0,0, np.zeros(shape=(500,500))

s0_getter_thread.start()
s1_getter_thread.start()
s2_getter_thread.start()
camera_getter_thread.start()
while True:
    try:
        s0_current=sq0.get_nowait()
    except queue.Empty:
        pass
    try:
        s1_current=sq1.get_nowait()
    except queue.Empty:
        pass
    try:
        s2_current=sq2.get_nowait()
    except queue.Empty:
        pass
    try:
        ret,frame=sqc.get_nowait()
        if ret:
            camera_current=frame
    except queue.Empty:
        pass
    draw_sensor_readings(camera_current,s0_current,s1_current,s2_current)
    window.show(camera_current)
    if cv.waitKey(1) == ord('q'):
        stop_threads=True
        break
s0_getter_thread.join()
s1_getter_thread.join()
s2_getter_thread.join()
camera_getter_thread.join()
window.__delete__(None)
camera.__delete__(None)
    

