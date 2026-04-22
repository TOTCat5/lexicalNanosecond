**"for"** is very close to the C++ version as it is written
"for(x in list)" or "for(i:int32 from 0 to 1000)".
It is important to note that the "from to" syntax is inclusive.
So a code such as "for(i:uint64 from to 1000){print(str(i));}" will print "012345678(...)9989991000",
if you want something closer to the usual "for(uint64_t i=0;i<1000;++i){printf("%zu",i)}" you can use the "from until" syntax which is basically the same as the "from to" but isn't inclusive of the last item