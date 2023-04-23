The goal of this work is to implement a hash table, compare efficency of different hash functions, then optimise this code using assembler optimization

## Hash function comparison
The following hash functions were compared:
1. 1; Just one.
2. first character (ASCII code)
3. length
4. Sum of ASCII codes
5. Start at 0. each step, rotate value left, then XOR it with next char. Return last value.
6. Same as 5, but rotate right instead of left
7. GNU hash (start at 5381, each step *33 and add next char)

Hash table is better with more even distribution of words between buckets. Let's analyze how this distribution affects performance. We expect that distribution of values we search for matches distribution of values in table. Actually it is not always the case, but it is too difficult to account for that. So, if we choose a random value, probability that it will fall into certain bucket is **(bucket size / total size)**. After the bucket was chosen, the weight of search in that bucket is also proportional to its size. So each bucket impacts performance as square of its size / total size.

This allows us to define a value, which can be used to compare hash functions:  
***badness*** = (sum of squared bucket sizes) / (total size)


1) is just nothing. Worst perfrmance, everything in 1 bucket.  
Badness: **8316** (equal to full size of table)

2) First char. Does help a bit, but is capped at 128 and ,because of difference in frequencies it causes strings to accumulate in specific buckets  
Badness: **~325**
![first chr diagram](run/pop_firstchr.png)

3) Length. Even worse, because there are just several typical lengths  
Badness: **~1091**
![length diagram](run/pop_len.png)

4) Sum. This gets a lot better. Howewer, it is not ideal: character codes used in text are distributed unevenly, so words tend to accumulate in groups with similar sums
Badness: **~15.4**
![sum diagram](run/pop_sum.png)

5) Rol+xor Good hash. Words are distributed quite evenly between buckets  
Badness: **~14.8**
![rol diagram](run/pop_rol.png)

6) Ror+xor Does not work well. The way lesser bits get shifted into higher ones caused some big periodic structures to appear.
(notice the scale)  
Badness: **~21.4**
![ror diagram](run/pop_ror.png)

7) GNU hash. Pretty good. Slightly better than rol+xor.  
Badness: **~14.6**
![gnuhash diagram](run/pop_gnu.png)

So, after examining different hash functions, GNU hash was chosen as the best candidate.

## Optimisations

First, we determine the main bottleneck by profiling code using callgrind. Kcachegrind was used to display the results. We optimise function "hashTableFind", so ignore everything outside. We see the following distribution of time: 
![profiler out 1](run/profile_1.png)

We can see that strcmp function has the highest perfomance impact. So it is the first optimisation point.
Good solution is to use avx instructions to compare strings instead of strcmp. This allows us to store test string (up to 32 chars) directly in avx register, and compare everythin with it with just a few instructions. Howewer, this does not work with strings longer then 31 characters, so set a fallback strcmp for that. It will not be used frequently because words we compare rarely exceed 31 chars. With this optimisations, we increase performance, but loose hardware compatibility. Also, code is linked to specific stored data type.


We see that it did help a bit. But things can be optimised even further. First, unaligned instructions were used to load each bucket value from memory. Second, D1 cache miss rate is quite high due to the lack of spatial locality (all strings are in random places of memory) Both these issues can be addressed by storing strings as si256 directly in data area of a bucket. The problem may arise that longer strings do not fit there, but it is easily solved using the last '\0' bytes of string as indicator that it is longer than normal and just store pointer in this case. Such pointer will never be equal to any normal string, and when comparing strings 'the old way' we check that byte to determine if it is pointer or not.