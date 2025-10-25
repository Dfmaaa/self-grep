# self-grep
ok so i didnt initially want to use multithreading,i just wanted to write something like grep, that could have input piped to it in a shell, i was going to 
use a simple buffer(at least that would've worked), but i decided to use a 'block' approach, I stored blocks of text in a linked list, one block of fixed length per node, the last one 
would have had variable length, so i kind of added support for variable buffer length(but it doesnt work so)

then i was like wait, what if reading input takes a lot of time, this issue worried me, because i've written programs that run for 3-4+ days, so i decided that we'll read input
and process input, at the same time, with a receiver thread, which receives data from stdin, and a finder thread, that looks for matches in the linked list. to avoid race conditions,
i added a block_complete field, so that the finder thread knows to wait, before accessing buffer of that node. 

After writing the receiver thread, i started thinking about the main program, and decided to write a seperate function _trv_blocks to traverse the list, you just had to say how many characters,
and the function would do the block changing for you. compartmentalization. writing that was the most tedious part. i also felt cool because this function has like 5 parameters.

and then later i finished writing code for the finder thread, to be honest, if it had worked first try, i would've been scared, because life would've found a way to maintain the 
balance. im glad it doesn't work. 

I was thinking of doing some frontend work(print the whole data, highlight the matches with red), but after getting a segfault, even after getting rid of the `6` bugs chatgpt told me
about, and it wasnt even slow, it didnt think, it was an immediate segfault. 

i have played these games before, im not debugging this hot mess. 

tldr: segfault
