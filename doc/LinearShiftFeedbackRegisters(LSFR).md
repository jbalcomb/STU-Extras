



the algebra of finite fields
- polynomials
- matrices


shift register
    a series of bit cells, each of which is a flip-flop
- it has an input signal
- it stores a single bit of state, either a 0 or a 1
- it changes that state bit to the input signal, upon receiving a clock signal
- it has an output signal that is the state of the shift register — if the shift register contains a 1, then it outputs a 1; if it contains a 0, then it outputs a 0

the cells are numbered from 0 to N−1 (here we have a 5-bit shift-register, so N=5)  

The state bits, collectively denoted S, are individually denoted S0 to SN−1 from right to left.  

The input to cell 0 is *u*, and the output of cell N−1 is *y*.  

We can denote the history of the input at discrete instances in time 0,1,2,…k by using brackets:  
Sj[k] represents the contents of bit j at timestep k, and u[k] and y[k] represent the input and output at timestep k.  


<i>u</i> the input to cell 0  
<i>y</i>the output of cell <i>N</i>−1  

<i>S</i><sub><i>j</i></sub>[<i>k</i>] represents the contents of bit <sub><i>j</i></sub> at timestep <i>k</i>,  
<i>u</i>[<i>k</i>] represent the input  
<i>y</i>[<i>k</i>] output  
at timestep <i>k</i>.  

<i>u</i>

<i>N</i>

<i>S</i>

<i>y</i>

<i>k</i>

<sub>j</sub>

<sub><i>j</i></sub>



https://www.embeddedrelated.com/showarticle/1065.php

They have many practical applications in communications theory.  
You can use LFSRs to generate a pseudorandom bit sequence, or as a high-speed counter.  
But part of the reason to understand the theory behind LFSRs is just because the math has an inherent beauty.

LINEAR FEEDBACK SHIFT REGISTERS: A BITWISE APPROACH
There are at least three ways to describe the linear feedback shift register, or LFSR:
a practical bitwise approach,
and two theoretical approaches based on the algebra of finite fields,
one using polynomials and
the other using matrices.
We’ll start with the bitwise approach.

A shift register is a series of bit cells, each of which is a flip-flop:
it has an input signal
it stores a single bit of state, either a 0 or a 1
it changes that state bit to the input signal, upon receiving a clock signal
it has an output signal that is the state of the shift register
 — if the shift register contains a 1, then it outputs a 1; if it contains a 0, then it outputs a 0
The output of one cell is connected to the input of the next,
  so that the bits in the shift register propagate
    from left to right, or right to left, depending on how the shift register is set up.


(There’s no strong reason to prefer right-to-left shifting rather than left-to-right shifting,  
  but it does correspond more nicely with some of the theoretical math if we do so.)  


The interesting stuff comes when we use feedback to alter the input or the state.  
LFSRs do this in one of two ways.  
- a Fibonacci representation LFSR
- a Galois representation LFSR

In a Fibonacci representation LFSR, the input *u* is set to the state bits XOR’d together  

In a Galois representation LFSR, the input *u* is set to 0, but some of the state bits are altered by XOR’ing in the output  



PROPERTIES OF THE LFSR
Okay, what do we get out of this thing?

With properly-chosen taps  
(and we’ll define more precisely what this means later),  
the LFSR is a maximal-length LFSR,  
and its output sequence can be considered a pseudo-random bit sequence (PRBS)  
which has these properties:  


there are abstract algebraic structures called groups, rings, and fields.  
If you want to be rigorous about analyzing them, you have to disregard any analogous real-world systems… but as mere mortal amateur mathematicians, that’s also the only way we can build intuition about them.  

So let’s start with a “times table”.  
