**Types** are one of the most importants things in a language.<br>
Without them,your code easily turn into a bloated mess that neither your ide or your ai agent can understand. ~~languageName~~ got you ! It has the usual: int8, uint8, int16, uint16, int32, uint32, int64, uint64, float, double, str and the good ol'char but also **Unsized** **Types** as int,uint and floaty ! Those types can only exist in instrincs declarations (ex:"pdep(a:int,b:int):int"). These are used when a instruction can take multiple word sizes but we can't have a version for evrey type for the same instruction can we ?<br>
<br>
**WARNING**: if you put types of different sizes it will directly cast the smaller ones to the biggest type.
