Multithreaded bit manipulation solution finder. The algorithm is fundamentally the same as all the rest, going through every possibile game state. 
It is very fast. Here's an example output, after compiling in VS Developer Console: `cl /O2 main.cpp` (optimizes for speed):  
`Enter number of simulations: 10000000`  
`22 threads will be used.`  
`Number of simulations per thread: 454545`
`Number of remainder simulations: 10`  
`Offset counter: 1 million`  
`Offset counter: 2 million`  
`Offset counter: 3 million`  
`Offset counter: 4 million`  
`Offset counter: 5 million`  
`Offset counter: 6 million`  
`Offset counter: 7 million`  
`Offset counter: 8 million`  
`Offset counter: 9 million`  
`Unsolvable percentage: 19.2214%`  
`Unsolvable count: 1922135`  
`Total time: 458480 milliseconds.`  
This is 7.6 minutes to evaluate 10 MILLION games. At that rate, 1 billion games could be solved per 12.67 hrs. Hell yeah
Next (and final stop) CUDA acceleration.
