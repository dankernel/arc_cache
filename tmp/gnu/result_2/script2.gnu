
#set terminal postscript enhanced mono
set term png
set terminal pngcairo size 800,600 enhanced font 'Verdana,14'
set output "gnuplot2.png"

#Style
set style data linespoints

#Title
set title "LRU cache hit ratio"
set title font ",20"

#Key
set key top

#Lable
set ylabel "Hit rato(%)"
set xlabel "Cache size(2^n)"
set ytics 0.1

#yrange
set yrange [13.7:14.2]

#Xtic rotate(Not do)
set xtic rotate by 0 scale 1

#plot for [i=2:3] 'result.dat' using i:xtic(i) title columnheader(i)
plot 'result2.dat' using 2:xtic(1) title columnheader(2)
set output
