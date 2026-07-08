using System.Runtime.InteropServices;

using RML.Interop;

using Xunit;

namespace Roblox.Tests;

public class VariantReaderTagValidationTests
{
    [Fact]
    public void Instance_Tagged_Variant_Requested_As_Int_Throws_InvalidCastException()
    {
        var variant = InteropVariant.FromPointer(0x1234);

        var ex = Assert.Throws<InvalidCastException>(
            () => Reflection.ConvertResult(variant, typeof(int), freeNativeResources: false));

        Assert.Contains("Instance", ex.Message);
        Assert.Contains("Int64", ex.Message);
    }

    [Fact]
    public void Int64_Tagged_Variant_Requested_As_String_Throws_InvalidCastException()
    {
        var variant = InteropVariant.FromInt64(42);

        var ex = Assert.Throws<InvalidCastException>(
            () => Reflection.ConvertResult(variant, typeof(string), freeNativeResources: false));

        Assert.Contains("Int64", ex.Message);
        Assert.Contains("String", ex.Message);
    }

    [Fact]
    public void Double_Tagged_Variant_Requested_As_Bool_Throws_InvalidCastException()
    {
        var variant = InteropVariant.FromDouble(3.5);

        var ex = Assert.Throws<InvalidCastException>(
            () => Reflection.ConvertResult(variant, typeof(bool), freeNativeResources: false));

        Assert.Contains("Double", ex.Message);
        Assert.Contains("Bool", ex.Message);
    }
}

public class VariantReaderTupleTests
{
    [Fact]
    public unsafe void Tuple_Of_Single_Blittable_Element_Is_Decoded_And_Freed()
    {
        var vectorBuffer = Marshal.AllocHGlobal(Marshal.SizeOf<Vector3>());
        var tupleBuffer = Marshal.AllocHGlobal(sizeof(ulong) + sizeof(InteropVariant));
        try
        {
            Marshal.StructureToPtr(new Vector3(1f, 2f, 3f), vectorBuffer, false);

            *(ulong*)tupleBuffer = 1;
            *(InteropVariant*)((byte*)tupleBuffer + sizeof(ulong)) = InteropVariant.FromBlittable((nuint)vectorBuffer);

            var tupleVariant = InteropVariant.FromTuple((nuint)tupleBuffer);

            var result = Reflection.ConvertResult(tupleVariant, typeof(Vector3), freeNativeResources: true);

            Assert.Equal(new Vector3(1f, 2f, 3f), Assert.IsType<Vector3>(result));
        }
        finally
        {
            Marshal.FreeHGlobal(vectorBuffer);
            Marshal.FreeHGlobal(tupleBuffer);
        }
    }

    [Fact]
    public unsafe void Tuple_With_Untyped_Blittable_Element_Frees_Buffer_And_Throws_Descriptive_Exception()
    {
        var vectorBuffer = Marshal.AllocHGlobal(Marshal.SizeOf<Vector3>());
        var tupleBuffer = Marshal.AllocHGlobal(sizeof(ulong) + 2 * sizeof(InteropVariant));
        try
        {
            Marshal.StructureToPtr(new Vector3(4f, 5f, 6f), vectorBuffer, false);

            *(ulong*)tupleBuffer = 2;
            var elements = (InteropVariant*)((byte*)tupleBuffer + sizeof(ulong));
            elements[0] = InteropVariant.FromInt64(7);
            elements[1] = InteropVariant.FromBlittable((nuint)vectorBuffer);

            var tupleVariant = InteropVariant.FromTuple((nuint)tupleBuffer);

            var ex = Assert.Throws<InvalidCastException>(
                () => Reflection.ConvertResult(tupleVariant, typeof(object[]), freeNativeResources: false));

            Assert.Contains("tuple", ex.Message, StringComparison.OrdinalIgnoreCase);
        }
        finally
        {
            Marshal.FreeHGlobal(vectorBuffer);
            Marshal.FreeHGlobal(tupleBuffer);
        }
    }
}

public class VariantReaderBinaryStringTests
{
    [Fact]
    public unsafe void BinaryString_Blittable_Variant_Decodes_To_ByteArray_Without_Throwing()
    {
        byte[] payload = [1, 2, 3, 4, 250];
        var buffer = Marshal.AllocHGlobal(sizeof(int) + payload.Length);
        try
        {
            Marshal.WriteInt32(buffer, payload.Length);
            Marshal.Copy(payload, 0, buffer + sizeof(int), payload.Length);

            var variant = InteropVariant.FromBlittable((nuint)buffer);

            var result = Reflection.ConvertResult(variant, typeof(byte[]), freeNativeResources: true);

            Assert.Equal(payload, Assert.IsType<byte[]>(result));
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }

    [Fact]
    public unsafe void Empty_BinaryString_Buffer_Decodes_To_Empty_Array()
    {
        var buffer = Marshal.AllocHGlobal(sizeof(int));
        try
        {
            Marshal.WriteInt32(buffer, 0);

            var variant = InteropVariant.FromBlittable((nuint)buffer);

            var result = Reflection.ConvertResult(variant, typeof(byte[]), freeNativeResources: false);
            Assert.Empty(Assert.IsType<byte[]>(result));
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }
}
