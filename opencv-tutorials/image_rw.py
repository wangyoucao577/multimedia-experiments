
import cv2 as cv
import sys

def main():
    img = cv.imread(cv.samples.findFile("1.jpg"))
    if img is None:
        sys.exit("Could not read the image.")

    cv.imshow("Display Window", img)
    k = cv.waitKey(0)
    
    if k == ord('s'):
        cv.imwrite("1-1.jpg", img)

if __name__ == "__main__":
    main()
