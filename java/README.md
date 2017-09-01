# Java port of TLSH

This folder contains a Java port of the Trend Micro Locality Sensitive Hash algorithm

## Use with gradle

Pre-built versions of TLSH are hosted on bintray and can be used in gradle build scripts as follows:

```
repositories {
    jcenter()

     maven {
         url  "https://dl.bintray.com/mrpolyonymous/tlsh"
     }
}

dependencies {
	compile 'com.trendmicro:tlsh:3.7.1'
	
    // ... other dependencies
}
```

**Note**: The process of getting the TLSH library in the main JCenter repository is under way.

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
