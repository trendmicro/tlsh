# Java port of TLSH

This folder contains a Java port of the Trend Micro Locality Sensitive Hash algorithm.

This port has no additional JAR dependencies and only requires the JRE.

## Use with gradle

Pre-built TLSH libraries will be available on Maven Central soon but for now they
must be built from source.

```bash
git clone https://github.com/trendmicro/tlsh.git
cd tlsh/java
./gradlew clean build
```

This will produce JAR files in `build/libs` that can be included in other projects.

## Example
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
