# Java port of TLSH

This folder contains a Java port of the Trend Micro Locality Sensitive Hash algorithm.

This port has no additional JAR dependencies and only requires the JRE.

## Use with gradle

Pre-built versions of TLSH are hosted on bintray and can be used in gradle build scripts as follows:

```
repositories {
  jcenter()
  // ... other repositories
}

dependencies {
  compile 'com.trendmicro:tlsh:3.7.1'

  // ... other dependencies
}
```

## Use with Maven

Pre-built versions of TLSH can be used in a Maven pom.xml file as follows:

```xml
<repositories>
  <repository>
    <id>jcenter</id>
    <url>https://jcenter.bintray.com/</url>
  </repository>
  <!-- ... other repositories -->
</repositories>

<dependencies>
  <dependency>
    <groupId>com.trendmicro</groupId>
    <artifactId>tlsh</artifactId>
    <version>3.7.1</version>
    <type>pom</type>
  </dependency>
  <!-- ... other dependencies -->
</dependencies>
```

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
