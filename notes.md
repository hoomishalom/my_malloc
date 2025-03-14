# NOTES

usage of get_overhead_size() is questionable and due to not knowing how the full project will look like
possibly screwed some of the statistics in meowheap

in conclusion it was easier than i thought, finished it in a couple of hours and most of the time was used to think of the archtecture of
the heap

note: i used next-fit packing heurestic due to ease of implementation altough using best-fit probably will be better, just means ill have to go through
the whole list of free chunks each time i allocate memory
