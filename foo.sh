function F() 
{ 
   if (( $1 == 0 )); then
      echo 1
      return
   fi
   x=$1
   echo $(( x * $(F $((x-1)) ) )) 
}
