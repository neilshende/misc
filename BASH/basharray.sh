for i in ${!myArray[@]}; do
  echo "element $i is ${myArray[$i]}"
done

for str in ${myArray[@]}; do
  echo $str
done
