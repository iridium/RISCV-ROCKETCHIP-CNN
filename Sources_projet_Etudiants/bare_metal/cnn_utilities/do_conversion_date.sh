total_imgs=`ls -1q image_inputs/* | wc -l`
input_images_list=`ls image_inputs/*`

echo "--------------------------------------"
echo $input_images_list
echo "--------------------------------------"

let "c=1"
for img in $input_images_list
do
    echo "Converting <$img>"

    python3 conversion.py $img date_in/$c.ppm 640 480

    ((c++))
done

# done

# for (( c=1; c<=$total_imgs; c++ ))
# do  
#    echo "Welcome $c times"
# done
