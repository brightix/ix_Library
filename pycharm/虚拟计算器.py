import cv2
import numpy as np
from cvzone.HandTrackingModule import HandDetector


class Button:
    def __init__(self,pos,width,height,value):
        self.pos = pos
        self.width = width
        self.height = height
        self.value = str(value)
        self.isClicked = False
        self.isRelease = True
    def draw(self,img):
        x,y = self.pos
        w,h = self.width,self.height
        if self.isClicked:
            x = x+3
            y = y+3
            w = w - 6
            h = h -6
            self.isClicked = False
        elif not self.isRelease:
            self.isRelease = True
        cv2.rectangle(img, (x, y), (x + w,y + h), (225, 225, 225), thickness=-1)
        cv2.rectangle(img, (x,y), (x + w,y + h), (0, 0, 0), thickness=2)

        text_size = cv2.getTextSize(self.value,cv2.FONT_HERSHEY_SIMPLEX,1,2)[0]
        text_x = int(x + (w - text_size[0])/2)
        text_y = int(y + (h + text_size[1])/2)
        cv2.putText(img, self.value, (text_x,text_y), cv2.FONT_HERSHEY_SIMPLEX, 1, (50, 50, 50), 2)
    def check_click(self,x,y):
        sx = self.pos[0]
        sy = self.pos[1]
        w = self.width
        h = self.height
        if sx < x < sx + w and sy < y < sy + h:
            self.isClicked = True

            if self.isRelease:
                self.isRelease = False
                return self.value
        return None


# cv2.namedWindow('window',cv2.WINDOW_NORMAL)
# cv2.resizeWindow('window',640,480)

cap = cv2.VideoCapture(0)
cap.set(3,720)
cap.set(4,480)

width = 60
height = 70

button_list = [
    Button((100 + x*width,100 + y*height),width,height,y*3 + x +1) for y in range(3) for x in range(3)
]

button_list.append(Button((280 + 0 * width, 100 + 0 * height), width, height, '+'))
button_list.append(Button((280 + 0 * width, 100 + 1 * height), width, height, '-'))
button_list.append(Button((280 + 0 * width, 100 + 2 * height), width, height, '*'))
button_list.append(Button((280 + 0 * width, 100 + 3 * height), width, height, '/'))
button_list.append(Button((280 - 3 * width, 100 + 3 * height), width, height, '='))
button_list.append(Button((280 - 2 * width, 100 + 3 * height), width, height, '0'))
button_list.append(Button((280 - 1 * width, 100 + 3 * height), width, height, '<'))




output_box = Button((100,100 - height),4*width,height,"")

#创建手部追踪
detector =  HandDetector(maxHands=2,detectionCon=0.8)

my_equation = ''

while True:
    flag,frame = cap.read()
    if not flag:
        exit()


    frame = cv2.flip(frame,1)

    hands,img = detector.findHands(frame,True,False)

    if hands:
        lmlist = hands[0]['lmList']
        # 提取食指和中指的 2D 坐标
        x1, y1,_ = lmlist[8]
        x2, y2,_ = lmlist[4]

        # 计算两点之间的距离
        length, info, frame = detector.findDistance((x1, y1), (x2, y2), frame)
        (x,y,_) = lmlist[8]
        if length < 40:
            for Button in button_list:
                my_value = Button.check_click(x, y)
                if my_value is not None:
                    if my_value == '=':
                        try:
                            my_equation = str(eval(my_equation))[:6]
                        except:
                            my_equation = ''
                    elif my_value == '<':
                        if my_equation:
                            my_equation = my_equation[:-1]
                    else:
                        my_equation += my_value
                    break

    #显示按钮

    for Button in button_list:
        Button.draw(frame)
    output_box.draw(frame)

    cv2.putText(frame,my_equation,(100,100),cv2.FONT_HERSHEY_SIMPLEX,2,(0,0,0),3)
    cv2.imshow('img',frame)
    key = cv2.waitKey(31)
    if key == ord('q'):
        break
    elif key == ord('c'):
        my_equation = ''
cap.release()
cv2.destroyAllWindows()