import cv2 as cv #Library for image processing
import sys  #Library system to retrieve the arguments 

def conversion(img_in,img_out,height,width):     
    image = cv.imread(img_in)  #Read the input image
    #cv.imshow('Normal',image)  #Display input image
    dim=(int(height),int(width))  #Values for resizing
    image = cv.resize(image,dim)  #Do the resize 
    #cv.imshow('Resize',image)    #Display resize image
    #cv.imwrite('resize.jpg',image)     #Creating new file with resizing image 
    image = cv.imwrite(img_out, image, (cv.IMWRITE_PXM_BINARY,0))  #Conversion in PPM 
    print("Image ",img_in,"converted in ppm. (",img_out,")")    #Output Message 

conversion(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4])    #Fonction execution
# In terminal : python conversion.py "img_in.jpg" "img_out.ppm" Height Width




