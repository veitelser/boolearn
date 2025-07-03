# User's Guide

This document comprises

* A brief description of **boolnets**, networks of Boolean threshold functions.

* Instructions for using the software package **boolearn** for machine learning on boolnets.


## Boolean threshold functions

A BTF has $m$ inputs and a single output. The inputs are Boolean, where `True/1` is encoded as $+1$ and `False/0` is encoded as $-1$. When combined as a vector $x$, the inputs have squared norm $x\cdot x = m$. Associated with each of the BTF's inputs is a real-valued weight. The corresponding $m$-component weight vector $w$ is also normalized as $w\cdot w = m$. The BTF's output, called $y$, is Boolean and given by the formula

$y = \mathrm{sgn}(w\cdot x)$

In boolearn this relationship is treated as a constraint. If it is not satisfied, all three entities ($w$, $x$, and $y$) are changed minimally, in a least-squares sense, so that the relationship holds.

Boolearn imposes an additional constraint that goes beyond how BTFs are normally used. This is a constraint on the margin between the two outputs:

$|w\cdot x| \ge \delta$

The positive number $\delta$ is the *margin parameter*. Large $\delta$ is more restrictive, and when sufficiently large, boolnets can represent arbitrary Boolean circuits comprised of binary `And/Or` gates as well as `Not`. Because weights of negative sign flip the truth value of the corresponding input, they are the `Not` gates in the circuit.

To better understand the Boolean circuit correspondence and its generalization, consider a boolnet that has been trained so the margin constraint, with some $\delta$, is satisfied at all the BTFs and over all the training data. By adjusting the weights at each BTF we can then increase $\delta$, individually for each BTF, without changing the Boolean function it implements. When $\delta$ is maximized, the weight vector has the form $(\sqrt{m/n})\mathbf{w}$, where $\mathbf{w}$ is a vector of $s$ nonzero coprime integers and squared norm $\mathbf{w}\cdot \mathbf{w}=n$, and there are $s$ linearly independent Boolean $x$ that satsify $|\mathbf{w}\cdot x|=1$. This $s=3$ example,

$w=(\sqrt{m/3}) (\pm 1, \pm 1, \pm 1, 0, \ldots, 0)$ ,

(and its permutations) has maximized margin $\delta=\sqrt{m/3}$. More generally (to get the largest $\delta$), the smallest $n(s)=\mathbf{w}\cdot \mathbf{w}$, when the integer vector $\mathbf{w}$ has support $s$, is $n(s)=s$ for odd $s$ and $n(s)=s-1$ when $s$ is even. In practical terms, this means that if we impose $\delta=\sqrt{m/n}$ on all the BTFs of the boolnet for some odd $n$, then all the BTFs are equivalent to maximized/rationalized BTFs of the kind described with support at most $s=n$, when $n$ is odd. This assumes of course that all the BTFs see $s$ linearly independent input tuples during training.

Boolnets trained with margin $\delta=\sqrt{m/3}$ can represent arbitrary logic circuits comprised of binary And/Or gates and Not. That's because the rationalized $w$ shown above implements a majority gate and

$\mathrm{And}(p,q) = \mathrm{Maj}(0,p,q)\qquad \mathrm{Or}(p,q) = \mathrm{Maj}(1,p,q)$

While these are sufficient to express arbitrary Boolean functions, the higher-support BTFs with smaller $\delta$ may offer more efficient representations. It's also important to bear in mind that the tight supports enabled by large $\delta$ get relaxed when BTFs do not see suffciently diverse inputs, as is often the case near the output end of a network.

## Directory organization

Boolearn has the following directories:

`autocode cellauto decode logic mnist mult src`

These hold data and results for various applications. All source files are kept in the directory src. To create the executables, go to src and do

`make all`

Since the executables are placed in that same directory, to run a program `prog` from one of the applications directories you need to do

`./../src/prog ...`

where `...` are the command-line arguments for `prog`. Since it's hard to remember all the arguments, all the programs are set up so that if you enter no arguments at all, the program tells you what they should be. 


## Networks

Boolnets can have the structure of any directed acyclic graph. The software does not expect layers. Networks are one of the main inputs/outputs and read/written to files with suffix `.net`. Because layered networks are so widely used, there is a program for generating such networks. To learn how to create a layered network and to see some small examples, go to the `mult` directory.

Data files have suffix `.dat`. Here's the top of `2x2.dat`:
```
16
2 4
2 4

1 0 0 1
0 0 1 0

0 1 1 0
0 0 1 0

etc.
```
The `16` in the header tells you there are $16$ data items. The `2 4` that follows tells you that the network expects $4$ binary (2-valued) inputs. Because of the `2 4` in the next line, the network produces $4$ binary outputs. From the two data items shown, you can see that this is just a collection of (2-bit) $\times$ (2-bit) multiplication facts (in some random order). For example, from the first two lines of data we learn that $2\times 1 = 2$, while the next two lines express $1\times 2 = 2$.

To learn the multiplication table up to 2-bits you need to create a network that takes $4$ Boolean inputs and produces the same number of outputs. The program `layered` will let you create such a network. Here is what happens when you make a query for the command-line arguments:
```
./../src/layered 
expected 3 arguments: andorprob widthfile netfile
```
The first argument is only relevant if you also want to set the network weights for generating synthetic data. We will use this feature in a different application. The second argument, `widthfile`, is a tiny file you need to create yourself that sets the widths of the layers. Here's an example, `2x2.wth`:
```
2
4 2 4
```
This defines a network with $2$ layers, $4$ input nodes, $4$ output nodes, and $2$ internal nodes. The last argument of `layered` lets you specify the network's name. For example,
```
./../src/layered .5 2x2.wth 2x2.net
```
produces the network `2x2.net`:
```
11  5  4  22
    5    0    0.00000000
    5    1    2.23606798
    5    2    0.00000000
    5    3    0.00000000
    5    4    0.00000000
    6    0   -1.29099445
    6    1   -1.29099445
    6    2   -1.29099445
    6    3    0.00000000
    6    4    0.00000000
    7    0    1.00000000
    7    5    1.00000000
    7    6    1.00000000
    8    0    0.00000000
    8    5   -1.73205081
    8    6    0.00000000
    9    0   -1.00000000
    9    5    1.00000000
    9    6   -1.00000000
   10    0   -1.00000000
   10    5    1.00000000
   10    6   -1.00000000
```
The first line tells you the network has $11$ nodes of which $5$ are inputs and $4$ are outputs. Since there are no BTFs at the inputs, the network has $11 - 5 = 6$ BTFs (one for each internal and output node). Why $5$ inputs instead of $4$? Node $0$ is an extra node not included in any layer that serves as a bias input to all the BTFs of the network. Its value is constant (`False/0`) and BTFs tune their bias by adjusting the corresponding weight. The program `layered` forms completely connected networks within each layer. The total number of edges in our example is therefore $6 + 2\times 4 + 4\times 2 = 22$.

In software, the plural forms of the graph objects are reserved for their number. Thus `nodes = 11`, `innodes = 5`, `outnodes = 4`, `edges = 22`.

The $22$ lines after the header give the node labels (`0` through `nodes-1`) for each edge. Labels `0` through `innodes-1` are reserved for the network inputs. The remaining nodes, having BTFs, are labeled so that by evaluating the corresponding BTFs in ascending order ($5, 6, \ldots, 10$), their inputs are either network inputs or the outputs of previously evaluated BTFs. The first node in each line of a network file is the receiving node, and explains why the lowest label is `5` in our example.

The numbers in the third column are the edge-weights. The program `train` we will use to learn the multiplication facts ignores these numbers. Still, we can check that the weight normalization is correct. The BTF at node `5` has five inputs with weights that have squared-norm $5$, etc. Moreover, each node has either $1$ or $3$ nonzero weights. The `andorprob` argument ($.5$) of `layered` was used to set the probability of the case $3$. When `layered` chooses (randomly) this case, the three weights are given equal magnitudes and random signs. Also, since the BTF in that case always receives input from the bias node `0`, the weights created by `layered` correspond to a random circuit of binary And/Or/Not gates. In a large network with setting `andorprob = .5`, about half of the nodes act as And/Or gates taking inputs from a random pair of nodes in the layer below. The other nodes, with a single nonzero weight, just move Boolean values (or their negation) between adjacent layers.

## First training example: a binary multiplier

You've taken a look at the data and already constructed a network that at least has the correct number of input and output nodes. Will it be able to learn $16$ multiplication facts? Of course not! The `2x2.net` you constructed has only $2$ hidden nodes, and can therefore produce at most $4$ distinct outputs (of the required $7$). Construct a new `2x2.net`, as before, but with $3$ layers and width $4$ for both hidden layers:
```
3
4 4 4 4
```
The training algorithm takes many arguments:
```
./../src/train 
expected 11 arguments: netfile datafile items btfn beta gam epochs itermax gapstop trials id
```
The argument names exactly match the variable names in the C program. Here's what they are asking for:
* `netfile` << `2x2.net`
* `datafile` << `2x2.dat`
* `items` << `16` (number of data you wish to train with)
* `btfn` << `3.` (sets the margin via the squared norm of the rationalized BTF weight vector, as described above)
* `beta` << `.2` (time-step value of the RRR constraint satisfaction algorithm)
* `gam` << `1e-3`(RRR metric auto-tuning parameter)
* `epochs` << 10 (maximum number of lines written to the file for monitoring progress)
* `itermax` << `1e4` (maximum number of RRR iterations)
* `gapstop` << `.01` (RRR termination criterion)
* `trials` << `1` (increase this when compiling statistics from multiple runs)
* `id` << `test` (name for all the files generated by `train`)

Because of the last argument, `train` will generate the following output files:
* `test.cmd` (your command line)
* `test.gap` (file for monitoring progress)
* `test.run` (statistics when doing multiple runs)
* `test.sol` (the solution weights)

The file whose contents you will be following closely, even as `train` is running, is the gap file, `test.gap`. For real-time reporting add an `&` at the end of your command line to put the job in the background. By doing
```
more test.gap
```
you will be able to follow the progress in long runs. The 2-bit binary multiplier application is so easy, `train` will have terminated by the time you get around to doing that. The complete gap file should look something like this:
```
         3    0.63613587    0.55792252    0.64555257    2.00700131    2.00700131    62.500 %     0.000 %
         7    0.28917853    0.43911374    0.46978192    1.28756702    1.28756702    64.062 %     0.000 %
        16    0.15349519    0.31768980    0.37626865    0.87848773    0.87848773    65.625 %     0.000 %
        40    0.12347367    0.18289424    0.30765258    0.59205003    0.56630854    73.438 %     0.000 %
       101    0.12159740    0.17687411    0.19909587    0.52520880    0.35032956    87.500 %     0.000 %
       252    0.05780539    0.10665988    0.10044701    0.29373600    0.23009609    84.375 %     0.000 %
       631    0.05448956    0.09306639    0.08987662    0.25398445    0.15553896    95.312 %     0.000 %
      1585    0.05765410    0.09512908    0.12523502    0.28574610    0.12876745    93.750 %     0.000 %
      3982    0.00343907    0.00408129    0.00714088    0.01291458    0.01001133   100.000 %     0.000 %
      4026    0.00221133    0.00248447    0.00637175    0.00984432    0.00984432   100.000 %     0.000 %
```
All the randomness in RRR comes at the start, when the weights ($w$), inputs ($x$), and outputs ($y$), are initialized. In `train` these are as random as possible: all are uniformly sampled between $-1$ and $+1$. Your `test.gap` will differ because of this.

The first column gives the RRR iteration count. If a solution had not been found after $10^4$ iterations (the setting for `itermax`) the last line would have had iteration count `10000`. In this run `train` terminated at iteration `4026`, because the lowest gap, shown in column 6, had reached the value of `gapstop` (`0.00984432 < .01`). We see that the training accuracy, shown in column 7, reaches 100 % a few iterations earlier. The generalization accuracy, in the last column, is shown as 0 % because all 16 multiplication facts were used in training, leaving none to test generalization.

Columns 2, 3, 4 show the gap broken down into contributions from the three variable types (in alphabetical order: $w$, $x$, $y$). These have a root-mean-square (rms) interpretation. For example, the `0.63613587` at the top of the second column means that in iteration 3, this was the rms difference in the constraint $A$ and constraint $B$ projections of the weights. To put this number in context, recall that the rms value of the weights is $1$ by our normalization choice. By iteration 4026 this number has decreased to `0.00221133`. The evolution of columns 3 and 4 is similar, as is the context ($x$ and $y$ are valued $\pm 1$). Column 5 is an aggregate rms gap at each BTF, where contributions from $w$, $x$, $y$ are combined. Column 6 is simply the lowest aggregate gap achieved.

In all cases (columns 2-6), when the rms gap goes to zero it means the $A$ and $B$ projections are nearly the same, or that a variable assignment has been found that satisfies both constraints. `train` uses the *divide-and-concur* constraint formulation where $A$ represents all the independent constraints at each BTF and each data item, while $B$ is responsible for concur, or the matching of one BTF's output ($y$) with another BTF's input ($x$), and the matching of the weights ($w$) across data items.

Entries to the gap file are added on a logarithmic scale, that is, the iteration count grows by a constant factor from one line to the next (`train` computes this factor from the `epochs` argument). Condensing the evolution to 10 or 50 lines of output makes it easier to digest the progress. As we will see in the more challenging applications, the convergence to zero gap is similar in character.

The solution weights are written to a file that has exactly the same format as the network file we created with `layered`. The only difference is that the weights in the third column are those determined by a constraint satisfaction algorithm. Here is the top of `test.sol`:
```
17  5  4  60
    5    0   -1.28983434
    5    1   -0.00200875
    5    2   -1.29054248
    5    3    0.01983954
    5    4   -1.29245110
    6    0    1.29876608
    6    1    0.00954843
    6    2    1.29683942
    6    3    1.27723133
    6    4    0.00177617
    7    0   -1.29377298
    7    1   -1.28797003
    7    2   -0.00451735
    7    3    0.01376609
    7    4   -1.29115250

etc.
```
We see that the BTFs at nodes 5, 6, and 7 (the only ones shown) are implementing Or gates. The small weights have no effect on the BTF outputs, and the near equality (in magnitude) of the others implies they will act exactly as Maj gates. In all three, one of the nonzero weights is to the bias input (node 0). The BTF at node 6 is therefore an Or, and by DeMorgan's rule, the BTFs at nodes 5 and 7 are And gates. That's because every BTF $f(x)$ is self-dual, or $f(x)=-f(-x)$, so that

$\neg \mathrm{Or}(\neg p,\neg q)=\neg \mathrm{Maj}(\neg 0,\neg p,\neg q)=\mathrm{Maj}(0,p,q)==\mathrm{And}(p,q)$.

Two kinds of symmetry act on the hidden nodes of layered boolnets. First, the nodes within each layer may be freely permuted (without changing the completely-connected, layered architecture). Second, because of self-duality, a boolnet's output is unchanged when the signs of all the incoming and outgoing weights, on any of the hidden BTFs, are flipped. This means that even when you expect unique solutions (say for a multiplier circuit), the weights in the solution file can and will come out differently from run to run because of symmetries. `train` calls the C time function to set the seed of the random number generator used for initialization.

Now that you know the basics of using `train`, there are many things to try, even for simple multiplier circuits. Try learning the 3-bit multiplication table, by constructing a circuit of only Maj gates with the setting `btfn << 3.` . The 64 multiplication facts are in the file `3x3.dat`. What are good candidates for the layered architecture? Wider layers, greater depth, both? After you succeed, you can try training on just a subset of the data, by setting `items` to a number less than 64. Now the generalization accuracy, in the last column of the gap file, will not be 0 %. Don't be disappointed when you find that you need to train on nearly all the data to get 100 % on the unseen multiplication facts. The complexity of multipier circuits grows almost as fast as the number of data. The right way to learn to generalize on binary multiplication is to put arbitrary-bit-length number representation into the model - something well beyond the scope of this tutorial! Generalizing in a different direction, experiment with larger values of `btfn`. Recall that larger squared-norms correspond to smaller margins, or less constrained, higher information-capacity networks. You will find that with `btfn << 5.` there exists a multiplier cicuit with only one hidden layer of 5 nodes. From the solution file you will see that the BTFs are now implementing both 3-input and 5-input majority gates. Because `btfn` is only used for setting the margin, it is not required to be an integer. Fractional values are fine.

## Revisiting the dawn of ML: the binary autoencoder

Rumelhart, Hinton and Williams, in the famous "backpropagation" paper, were justifiably excited by the fact that their gradient descent algorithm was able to find weights that allowed information to pass through a bottleneck. Their network, called an *autoencoder*, was challenged with mapping $2^n$ random vectors to themselves, from $2^n$ input nodes to $2^n$ output nodes, but through a layer of only $n$ hidden nodes. This is impossible without nonlinearity in the activation functions, and backpropagation's ability to solve the nonconvex optimization problem was a triumph. Because sigmoids were used for the activation functions, which saturate to the values 0 and 1, the authors remarked that the training had succeeded in encoding the random vectors into binary codewords, and then decoding these back to the original vectors. We are revisiting the binary autoencoder because the gradient-descent-trained networks do not in fact deliver that result: the codes (outputs of the $n$ sigmoids in the bottleneck) are *not* binary.

Since constraint-based methods were not well known in 1985, Rumelhart et al. can't be faulted for using gradient-based methods, where 0 and 1 lie in a continuum. Boolnets do not have that limitation, and by construction deliver true binary encodings/decodings if all the constraints can be satisfied. Move into directory `autocode` to try it out yourself.

Three sets of profoundly silly data files are provided, corresponding to $n=3, 4, 5$. We will experiment with `auto16.dat`, the top of which is copied below:
```16
2 16
2 16

1 1 1 1 0 1 1 0 1 1 1 0 0 1 0 0
1 1 1 1 0 1 1 0 1 1 1 0 0 1 0 0

0 1 0 0 0 0 0 0 1 1 1 0 0 0 1 1
0 1 0 0 0 0 0 0 1 1 1 0 0 0 1 1
```
Two of the 16 matching input/output vectors are shown. Rumelhart et al. used 1-hot vectors (and $2^3=8$). Our vectors are more random. A simple 2-layer architecture with layer-widths
```
2
16 4 16
```
works for 1-hot vectors. For our data we create a network `auto16.net` having additional layers in both the encoder and decoder stages:
```
4
16 16 4 16 16
```

We will use this application to demonstrate basic hyperparameter optimization techniques. `train`, and the RRR algorithm more generally, has two hyperparameters: `beta` ($\beta$) and `gam` ($\gamma$). $\beta$ is like the gradient-step or learning-rate parameter of gradient descent. However, because RRR is *not* approximating continuous dynamics, large values like $\beta=1$ make perfect sense. In fact, RRR is the "relaxation" of the $\beta=1$ algorithm, better known as the popular "Douglas-Rachford" algorithm for convex problems. Experiments have shown that small $\beta$ helps in nonconvex problems.

To see the effects of $\beta$, we launch two runs of `train` like this:
```
./../src/train auto16.net auto16.dat 16 3. .2 1e-4 10 1e5 .05 5 test1 &

./../src/train auto16.net auto16.dat 16 3. .5 1e-4 10 1e5 .05 5 test2 &
```
These differ only in the value of $\beta$, `.2` (`test1`) and `.5` (`test2`). Some of the arguments, like `gapstop`, have been changed from what we used in the multiplier application. A new feature is enabled by the setting `trials` << `5`. The results of 5 runs, differing only in the random initialization, are summarized in the two run files. Here is `test1.run`:
```
   1       90291    0.04924928   100.000 %     0.000 %
   2       73029    0.04878647   100.000 %     0.000 %
   3       57484    0.04985247   100.000 %     0.000 %
   4       37528    0.04934596    99.219 %     0.000 %
   5       17242    0.04877118   100.000 %     0.000 %

5 / 5 solutions   55114.800000 +/- 11502.245400 iterations/solution
```
Compare this with `test2.run`, the experiments with the *larger* $\beta$ :
```
   1           0    0.18900071   100.000 %     0.000 %
   2           0    0.17517734    99.609 %     0.000 %
   3           0    0.25684861    95.312 %     0.000 %
   4           0    0.22922137    97.656 %     0.000 %
   5           0    0.30500119    93.359 %     0.000 %

 no solutions found
```
Without knowing what the columns mean, we see that smaller $\beta$ is better. All five runs in `test1` completed because the gap, shown in column 3, went below `gapstop`. The iteration counts are given in column 2. As in gap files, the two columns at the far right give the accuracies, "training" followed by "generalization" (0 % here because we trained on all the data). Notice that the training accuracy in run 4 is not perfect, even though the `gapstop` termination criterion was achieved. This can be fixed by decreasing `gapstop`, which will also require increasing `itermax`. The iteration count for non-solution runs, as in `test2`, are recorded as 0.

The same approach can be used to find a good value for the metric auto-tuning parameter $\gamma$. The projections to constraints $A$ and $B$ at the core of the RRR algorithm minimize distances to $A$ and $B$, seen as sets in Euclidean space. Initially it's fine to use the same metric coefficients on all the variables when computing projections. However, over the course of a run it will usually be the case that some BTF, or some BTF faced with a particular training item, is more frustrated than its cohort in closing the gap - the distance between the $A$, $B$ projections of its variables. Since local optimizations often fail in nonconvex problems, the $\gamma$ hyperparameter was introduced to allow the metric coefficients to respond to gap heterogeneity. It's a heuristic that stiffens the metric on variables with a large gap, while doing the opposite with the small-gap variables, thereby shifting the job of constraint satisfaction to variables that haven't been changing much (and have small gaps). $\gamma$ sets the rate at which the metric relaxes to the nonuniform form derived from the gaps. It should be set at a small value, so as not to upset the local convergence properties of RRR. A good rule of thumb is to use a value such that `gam` $\times$ `itermax` is of order 1.

## Decoding and generative models

The decoder-half of an autoencoder is a primitive *generative model* : from just a few bits of input, arbitrarily complex (perhaps even interesting!) data vectors (text, images) can be conjured up. As a warm-up, we'll try `train` on some small random vectors, like those in the autoencoder experiments. After some experience with that, we'll move on to some images of the number 4 from the internet.

In the directory `decode` you will find the file `rand16.dat`:
```
16
2 0
2 16

0 0 0 0 1 0 0 1 1 1 1 1 1 1 0 1

0 0 1 1 0 0 1 1 0 1 1 0 1 0 0 1

etc.
```
Shown above are just two of the 16 data items (random vectors). The `2 0` and `2 16` in the header tells `train` there is no data to constrain the network inputs and there are 16 binary data that constrain the outputs (the random vectors). Thanks to the constraint formulation, `train` can work with missing inputs in much the same way it works with known inputs. For the $A$ constraint, instead of projecting node variables $y$ at the input nodes to known values ($-1$ or $+1$), the projection replaces each $y$ by *either* $-1$ or $+1$, whichever is closer (has the shorter projection distance). Everything else (projections to all the BTF constraints, concur) is unchanged.

Recalling what we found in the autoencoder experiments, it should be possible to construct a boolnet that decodes 4 bits into 16 random vectors with the following layered architecture:
```
2
4 16 16
```
Using the `rand16.net` constructed by `layered` with this width file, we run `train` as in the other examples:
```
./../src/train rand16.net rand16.dat 16 3. .5 1e-4 10 1e6 .01 1 test &
```
Here is `test.gap`:
```
         4    0.13255935    0.22712855    0.87000768    1.22930747    1.22930747     0.000 %     0.000 %
        16    0.10139585    0.19051218    0.60907604    0.92662968    0.92662968     0.000 %     0.000 %
        64    0.07857407    0.12233721    0.43314459    0.63719458    0.59279208     0.000 %     0.000 %
       252    0.04567090    0.06724448    0.28383838    0.37568461    0.31569327     0.000 %     0.000 %
      1001    0.03200167    0.05656542    0.19654548    0.27119040    0.24861232     0.000 %     0.000 %
      3982    0.03758419    0.06244747    0.17790543    0.29341729    0.19169208     0.000 %     0.000 %
     15849    0.02993051    0.04959298    0.11080268    0.22515698    0.10169874     0.000 %     0.000 %
     63096    0.04092607    0.06941564    0.13707712    0.30456958    0.04868250     0.000 %     0.000 %
    122167    0.00193576    0.00154775    0.00604958    0.00991337    0.00991337     0.000 %     0.000 %
```
The small final gap leaves no doubt that a true decoder network has been found, even when that can't be confirmed by the accuracies (rightmost columns) when there is no input data. However, `train` provides a check by writing generator files when there is no input data. Because we used `test` for `id`, the generator file is `test.gen`. Here is how it begins:
```
16  4

 0 0 0 0
 1 0 0 0
 0 1 0 0
 1 1 0 0
 0 0 1 0

etc.
```
Since our network architecture specified 4 input/code bits, each line has a binary assignment to those bits. Moreover, for the network to generate all 16 (distinct) random vectors, all $2^4$ assignments must be used. `train` lists them as increasing binary numbers.

The most direct way to check our decoder is with the synthetic data generating program `data`. Here is what we get when we query its arguments:
```
./../src/data
expected 4 arguments: netfile genfile items id
```
For `netfile` we should *not* use `rand16.net`, since `layered` created that file with random weights (that did not get used by `train`). Instead, we should use `test.sol`: the same network but with solution weights. The next argument for `data` is the generator file that goes with the solution, `test.gen`. The argument `items` is needed when we want `data` to generate synthetic data (`items` in number) by sampling random inputs. Since here the file `test.gen` provides all the inputs, we tell `data` to skip the random data generation with the setting 0 for this argument. The command
```
./../src/data test.sol test.gen 0 test
```
creates the data file `test.dat` :
```
16
2 4
2 16

 0 0 0 0
 0 0 0 0 1 0 0 0 1 0 1 1 0 1 0 1

 1 0 0 0
 1 0 0 1 1 1 0 1 1 1 0 1 1 0 1 0

 0 1 0 0
 1 1 1 1 1 1 0 1 1 1 1 1 0 0 1 1

 1 1 0 0
 1 1 1 0 1 1 0 1 1 0 0 0 1 0 1 1

 0 0 1 0
 0 0 0 0 1 0 0 1 1 1 1 1 1 1 0 1

etc.
```
All the random vectors of `rand16.dat` appear somewhere in the list, now preceded by the decoder inputs. In generative models the symmetries that previously only applied to the hidden nodes also apply to the input nodes. That explains why the first random vector of `rand16.dat` can end up as the 5th item in `test.dat`.
