import cv2
import numpy as np
import argparse

parse = argparse.ArgumentParser()
parse.add_argument('-i','--image',required=True,help='path to input image')
parse.add_argument('-t','--template',required=True,help='path to template ocr image')

tmp = parse.parse_args()
args = vars(tmp)

ref = cv2.imread(args['template'])
ref_w,ref_h = ref.shape[:2]
mask = np.zeros((ref_w,ref_h),dtype = np.uint8)

# mask[800:ref_w-10,10:ref_h-10] = 255
# ref = cv2.bitwise_and(ref,ref,mask=mask)
ref_gray = cv2.cvtColor(ref,cv2.COLOR_BGR2GRAY)

kernel = cv2.getStructuringElement(cv2.MORPH_RECT,(5,5))
ref_gray = cv2.morphologyEx(ref_gray,cv2.MORPH_OPEN,kernel)


_,ref_gray = cv2.threshold(ref_gray,10,255,cv2.THRESH_BINARY_INV)

ref_contours , _ = cv2.findContours(ref_gray,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
cv2.drawContours(ref,ref_contours,-1,(0,0,255),3)

#数字排序为升序
bounding_boxes = [cv2.boundingRect(c) for c in ref_contours]
#bounding_boxes = sorted(bounding_boxes,key=lambda x:x[0])
(ref_contours,bounding_boxes) = zip(*sorted(zip(ref_contours,bounding_boxes),key=lambda b:b[1][0]))
# print((ref_contours,bounding_boxes))

digits = {}

for (i,c) in enumerate(ref_contours):
    (x,y,w,h) = cv2.boundingRect(c)
    roi = ref_gray[y:y+h,x:x+w]
    roi = cv2.resize(roi,(57,88))
    digits[i] = roi
################################
img = cv2.imread(args['image'])

h,w = img.shape[:2]
nw = 300
img = cv2.resize(img,(nw,int(h*nw/w)))

img_gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)


rect_kernel = cv2.getStructuringElement(cv2.MORPH_RECT,(9,3))

tophat = cv2.morphologyEx(img_gray,cv2.MORPH_TOPHAT,rect_kernel)



grad_x = cv2.Sobel(tophat,ddepth=cv2.CV_32F,dx=1,dy=0,ksize=-1)

grad_x = np.absolute(grad_x)
min_val,max_val = np.min(grad_x),np.max(grad_x)
grad_x = ((grad_x - min_val) / (max_val - min_val)) * 255
grad_x = grad_x.astype('uint8')


# grad_y = cv2.Sobel(tophat,ddepth=cv2.CV_32F,dx=0,dy=1,ksize=-1)
# grad_y = np.absolute(grad_y)
# min_val,max_val = np.min(grad_y),np.max(grad_y)
# grad_y = ((grad_y - min_val) / (max_val - min_val)) * 255
# grad_y = grad_y.astype('uint8')


grad_x = cv2.morphologyEx(grad_x,cv2.MORPH_CLOSE,rect_kernel)




_,thresh = cv2.threshold(grad_x,0,255,cv2.THRESH_BINARY | cv2.THRESH_OTSU)


sq_kernel = cv2.getStructuringElement(cv2.MORPH_RECT,(5,5))

thresh = cv2.morphologyEx(thresh,cv2.MORPH_CLOSE,sq_kernel)



thresh_contours,_ = cv2.findContours(thresh.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
img_copy = img.copy()
cv2.drawContours(img_copy,thresh_contours,-1,(0,0,255),2)

cv2.imshow('img_copy',img_copy)

locs = []
for c in thresh_contours:
    (x,y,w,h) = cv2.boundingRect(c)
    ar = w/float(h)

    if 2.5 < ar < 4.0:
        if 55 > w > 40 and 20 > h > 10:
            locs.append((x,y,w,h))
sorted(locs, key=lambda xx:xx[0])

for(i,(gx,gy,gw,gh)) in enumerate(locs):
    border = 3
    group = img_gray[gy-border:gy+gh+border,gx-border:gx+gw+2*border]
    group = cv2.threshold(group,10,255,cv2.THRESH_BINARY | cv2.THRESH_OTSU)[1]
    digit_contours,_ = cv2.findContours(group.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)

    bounding_boxes = [cv2.boundingRect(c) for c in digit_contours]
    (digit_contours,_) = zip(*sorted(zip(digit_contours, bounding_boxes), key=lambda b: b[1][0]))
    group_output = []
    for c in digit_contours:
        (x,y,w,h) = cv2.boundingRect(c)
        roi = group[y: y + h,x: x + w]
        roi = cv2.resize(roi,(57,88))
        scores = []
        for(digit,digit_roi) in digits.items():
            result = cv2.matchTemplate(roi,digit_roi,cv2.TM_CCOEFF)
            (_,score,_,_) = cv2.minMaxLoc(result)
            scores.append(score)
        group_output.append(str(np.argmax(scores)))
    cv2.rectangle(img,(gx-5,gy-5),(gx+gw+5,gy+gh+5),(0,0,255),1)
    cv2.putText(img,''.join(group_output),(gx,gy-15),cv2.FONT_HERSHEY_SIMPLEX,0.6,(0,0,255),2)



cv2.imshow('window',img)



cv2.waitKey(0)
cv2.destroyAllWindows()