import cv2 as cv

cap = cv.VideoCapture("/dev/video0")
while True:
    ret, frame = cap.read()
    cv.imshow("test",frame)
    if cv.waitKey(1) == ord('q'):
        break