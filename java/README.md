# Java port of TLSH

This folder contains a Java port of the Trend Micro Locality Sensitive Hash algorithm

Example usage:
```java
import com.trendmicro.tlsh.Tlsh;
import com.trendmicro.tlsh.TlshCreator;


TlshCreator tlshCreator = new TlshCreator();
byte[] buf = new byte[1024];
InputStream is = ...; // however you get your input
int bytesRead = is.read(buf, 0, buf.length);
while (bytesRead >= 0) {
    tlshCreator.update(buf, 0, bytesRead);
   bytesRead = is.read(buf, 0, buf.length);
}
is.close();
Tlsh hash = tlshCreator.getHash();
System.out.println("Hash is " + hash.getEncoded());

Tlsh otherHash = ...; // compute another hash
int diff = hash.totalDiff(otherHash, true);
System.out.println("TLSH difference between data is " + diff);
```
