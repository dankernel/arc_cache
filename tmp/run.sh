
file_name=`basename $1 .txt`

./main $1 1 > "out/"$file_name"_1.out" & \
  echo "end 1" & \
./main $1 2 > "out/"$file_name"_2.out" & \
  echo "end 2" & \
./main $1 2 > "out/"$file_name"_4.out" & \
  echo "end 4" & \
./main $1 8 > "out/"$file_name"_8.out" & \
  echo "end 8" && \
./main $1 16 > "out/"$file_name"_16.out" & \
  echo "end 16" & \
./main $1 32 > "out/"$file_name"_32.out" & \
  echo "end 32" && \
./main $1 64 > "out/"$file_name"_64.out" && \
  echo "end 64" && \
./main $1 128 > "out/"$file_name"_128.out" && \
  echo "end 128" && \
./main $1 256 > "out/"$file_name"_256.out" && \
  echo "end 256" && \
./main $1 512 > "out/"$file_name"_512.out" && \
  echo "end 512"
