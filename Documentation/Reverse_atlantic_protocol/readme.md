Those file were captured using DreamSourceLab logic analyser and can be open in the DSView software (freely available at https://www.dreamsourcelab.com/download/)

As those format are not completely standard IR (looking like SIRC but not the same frame format nor the same bit timing), a decoder is provided in sources/tool folder of this project. The folder ir_SIRC_Atlantic shall be pasted in the Decoders folder of DSview installation folder. For me it is C:\Program Files\DSView\decoders.

This decoder is a dirty evolution from the standard SIRC decoder provided.
With minor adaptation if any, this decoder should also work with GNU PulseView (https://sigrok.org/wiki/PulseView)