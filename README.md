
https://www.hackster.io/whitney-knitter/dsp-for-fpga-simple-fir-filter-in-verilog-91208d#overview

This repository is designed to hold a variety of demonstration filters.
These filters will be discussed and used as examples on
the [ZipCPU blog at zipcpu.com](http://zipcpu.com).  If you watch carefully,
you may find filters here before they are posted, as I'm going to be doing my
development here.  Still, there have been many posts already that you may
find valuable.  These include:

1. A description (and implementation of) the two [simplest filters](http://zipcpu.com/dsp/2017/08/19/simple-filter.html) I know of.

1. A [Generic FIR](http://zipcpu.com/dsp/2017/09/15/fastfir.html) implementation

1. A [Simpler Generic FIR](http://zipcpu.com/dsp/2017/09/15/cheaper-fast-fir.html) implementation

1. A [Moving Average/Boxcar Filter](http://zipcpu.com/dsp/2017/10/16/boxcar.html)

1. A [Linear Feedback Shift Register (LFSR)](http://zipcpu.com/dsp/2017/10/27/lfsr.html)

1. [Building a generic filtering test harness](http://zipcpu.com/dsp/2017/11/04/genfil-tb.html)
   - [Measuring a filter's frequency response](http://zipcpu.com/dsp/2017/11/22/fltr-response.html)

1. [Delaying elements in a DSP system](http://zipcpu.com/dsp/2017/11/10/delayw.html)

1. [Generating a Pseudorandom noise stream via an LFSR](http://zipcpu.com/dsp/2017/11/11/lfsr-example.html)

   - An [Example LFSR Output](http://zipcpu.com/dsp/2017/11/11/lfsr-example.html)

   - And [How to generate multiple bits per clock using an LFSR](http://zipcpu.com/dsp/2017/11/13/lfsr-multi.html)

1. [Testing a generic filter using the test harness](http://zipcpu.com/dsp/2017/12/06/fastfir-tb.md)

1. [Building a slower filter](http://zipcpu.com/dsp/2017/12/30/slowfil.md),
   one that time-multiplexes a single one hardware multiply across many
   coefficients.

I have other filters built as well that I am looking forward to both
writing about and adding to this repository.  These include a
symmetric filter, a halfband filter, or even a Hilbert transform.  I've
also got an example filter implementation that runs between system clocks,
or another which allows a larger filter to be created from a simple cascade
of smaller filter components--each of which runs between clock steps.
These filters are likely to join the ones already present within this
repository.

I also have code to support testing a filter in-situ, rather than just
with Verilator.  Whether or not I get to the point of presenting this code
will need to be determined.

# License

This repository is released under the terms and conditions of the
[LGPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html).  If these conditions
are not sufficient for you and your purposes, other licenses terms are
available for a minimal purchase price.
