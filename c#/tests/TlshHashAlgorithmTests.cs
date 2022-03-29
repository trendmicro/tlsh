using System;
using System.IO;
using System.Security.Cryptography;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Shouldly;

namespace TrendMicro.Tlsh;

[TestClass]
public class TlshHashAlgorithmTests
{
	[TestMethod]
	[DataRow(BucketOption.Default, ChecksumOption.OneByte)]
	[DataRow(BucketOption.Default, ChecksumOption.ThreeBytes)]
	[DataRow(BucketOption.Extended, ChecksumOption.OneByte)]
	[DataRow(BucketOption.Extended, ChecksumOption.ThreeBytes)]
	public void TestCryptoTransform_Read(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		var (data, expected) = GetTestData(bucketOption, checksumOption);

		var hash = new TlshHashAlgorithm(bucketOption, checksumOption, VersionOption.Version4);

		var source = new MemoryStream(data);
		var dest = new MemoryStream();
		var stream = new CryptoStream(source, hash, CryptoStreamMode.Read);
		stream.CopyTo(dest);

		dest.ToArray().ShouldBe(data);

		hash.Hash.ShouldBe(expected.ToByteArray());
		AssertHashSize(hash.HashSize, bucketOption, checksumOption);
	}
	
	[TestMethod]
	[DataRow(BucketOption.Default, ChecksumOption.OneByte)]
	[DataRow(BucketOption.Default, ChecksumOption.ThreeBytes)]
	[DataRow(BucketOption.Extended, ChecksumOption.OneByte)]
	[DataRow(BucketOption.Extended, ChecksumOption.ThreeBytes)]
	public void Test_ComputeHash(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		var (data, expected) = GetTestData(bucketOption, checksumOption);
		var hash2 = new TlshHashAlgorithm(bucketOption, checksumOption, VersionOption.Version4);

		var actual = hash2.ComputeHash(data);
		actual.ShouldBe(expected.ToByteArray());
	
		AssertHashSize(hash2.HashSize, bucketOption, checksumOption);
	}

	[TestMethod]
	public void Test_NotEnoughData()
	{
		var data = new byte[60];
		var rng = new Random();
		rng.NextBytes(data);
		var hash = new TlshHashAlgorithm();

		var actual = hash.ComputeHash(data);
		actual.ShouldBeEmpty();
		AssertHashSize(hash.HashSize, BucketOption.Default, ChecksumOption.OneByte);
	}

	private static void AssertHashSize(int hashSize, BucketOption bucketOption, ChecksumOption checksumOption)
	{
		hashSize.ShouldBe((bucketOption, checksumOption) switch
		{
			(BucketOption.Default, ChecksumOption.OneByte) => 280,
			(BucketOption.Extended, ChecksumOption.OneByte) => 536,
			(BucketOption.Default, ChecksumOption.ThreeBytes) => 296,
			(BucketOption.Extended, ChecksumOption.ThreeBytes) => 552
		});
	}

	private static (byte[] Data, TlshValue ExpectedResult) GetTestData(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		var data = new byte[1024];
		var rng = new Random();
		rng.NextBytes(data);
		var hash = new Tlsh(bucketOption, checksumOption, VersionOption.Version4);
		hash.Update(data);
		hash.TryGetHash(out var expected).ShouldBeTrue();
		return (data, expected);
	}
}