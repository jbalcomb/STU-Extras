
SEEALSO:  Graphics Gems (1990) 
J:\STU_J\__STU-EduMat\STU-EduMat\_Graphics_VGA\Graphics Gems (1990)\GraphicsGems-master\gems\
Dissolve.c
IV.3
A DIGITAL "DISSOLVE" EFFECT
Mike Morton
Honolulu, Hawaii
Page 221  (PDG Page 242)
Randomly Traversing a 2D Array
Scrambling Integers in Hardware
Scrambling Integers in Software
Table 1.





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









https://www.dcode.fr/linear-feedback-shift-register



Answers to Questions (FAQ)

What is a linear feedback shift register? (Definition)
A linear feedback shift register or LFSR is a system that generates bits from a register and a feedback function.

After several iterations, the register returns to a previous state already known and starts again in a loop, the number of iterations of which is called its period.

In practice, this kind of LFSR register is useful in cryptography because it allows the generation of pseudo-random numbers when the period is long enough.

How does a LFSR work?
From the current state of the binary register, a bit selection is chosen to apply an XOR operation to it.

The result is then concatenated at the start of the register (on the left) while the final value (on the right) is deleted from the register and displayed at the output.

Example: The register is 1010, an XOR ⊕ is applied to bits 0 and 2 (numbered from left to right starting from 0)

Step  Register     XOR         Output
  0   _______0101  0 ⊕ 0 = 0  
  1   ______0010_  0 ⊕ 1 = 1  1
  2   _____1001__  1 ⊕ 0 = 1  0
  3   ____1100___  1 ⊕ 0 = 1  1
  4   ___1110____  1 ⊕ 1 = 0  0
  5   __0111_____  0 ⊕ 1 = 1  0
  6   _1011______  0 ⊕ 1 = 1  1
  7   0101_______  0 ⊕ 0 = 0  1

How to use a LFSR?
To use an LFSR, the user must configure the register length, determine the initial configuration of the flip-flops, choose an appropriate linear feedback function (XOR), and specify the number of iterations/cycles needed to generate the desired sequence.

How to calculate the period of an LFSR?
The period of an LFSR is the number of iterations necessary for the generated sequence to return to its initial state.

If the register is at maximum feedback, there is no repetition, except after 2^n - 1 iteration, with n the length of the register.







int16_t Random__FAIL(int16_t max)
{
    int16_t itr;
    int16_t result;

    // The number of bits in the shift register.
    #define N 16

    // The initial state of the shift register.
    uint16_t state = 0xACE1;

    // The feedback polynomial.
    // uint16_t feedback = 0xB400;
    uint16_t feedback = 0x6A00;

    // The output bit.
    uint16_t output;

    // The next state of the shift register.
    uint16_t next_state;


    // Initialize the shift register.
    // state = 0xACE1;
    state = random_seed;


    if(max == 0)
    {
        // GAME_QuitProgram(cnst_RND_Error);  /* "RND no 0's" */
        // MoO2  return 1;
    }

    output = 0;

    // Loop through 8-bits
    itr = 9;
    while(itr--)
    {
        // Generate the output bit.
        output = state & 1;

        // Shift the shift register.
        next_state = (state >> 1) ^ (output & feedback);

        // Update the state of the shift register.
        state = next_state;

    }

    result = output % max;

    return result;
}



/*
http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html

/ * 16-bit xorshift PRNG * /

unsigned xs = 1;

unsigned xorshift( )
{
    xs ^= xs << 7;
    xs ^= xs >> 9;
    xs ^= xs << 8;
    return xs;
}
*/
