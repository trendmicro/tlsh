using System.Security.Cryptography;

namespace TrendMicro.Tlsh
{
	/// <summary>
	/// Computes the TLSH hash for the input data.
	/// </summary>
	public class TlshHashAlgorithm : HashAlgorithm
	{
		private readonly Tlsh _Tlsh;
		private readonly BucketOption _BucketOption;
		private readonly ChecksumOption _ChecksumOption;

		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using recommended default values.
		/// </summary>
		public TlshHashAlgorithm(): this(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4) { }

		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using the specified values.
		/// </summary>
		/// <param name="bucketOption">Specifies the bucket count; more buckets can give better difference calculations but require more space to store the computed hash.</param>
		/// <param name="checksumOption">Specifies the checksum length; longer checksums are more resilient but increase hash creation time.</param>
		/// <param name="versionOption">The version of TLSH to use. It is usually best to use the latest version, unless you need an older version for compatibility reasons.</param>
		public TlshHashAlgorithm(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption)
		{
			_Tlsh = new Tlsh(bucketOption, checksumOption, versionOption);
			_BucketOption = bucketOption;
			_ChecksumOption = checksumOption;
		}

		/// <inheritdoc />
		public override int HashSize => ((int) _ChecksumOption + 1 + 1 + (int) _BucketOption / 4)*8;

		/// <inheritdoc />
		protected override void HashCore(byte[] array, int ibStart, int cbSize) => _Tlsh.Update(array, ibStart, (uint) cbSize);

		/// <inheritdoc />
		protected override byte[] HashFinal() => _Tlsh.GetHash().ToByteArray();

		/// <inheritdoc />
		public override void Initialize() => _Tlsh.Reset();
	}
}