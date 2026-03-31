namespace RML.Interop;

public static unsafe class Interop
{
    public static bool IsInitialized => Table != null;

    internal static NativeInterop.InteropTable* Table { get; private set; }

    public static void Initialize(nint tablePtr)
    {
        if (tablePtr == nint.Zero)
        {
            throw new ArgumentException("Invalid interop table pointer or size.");
        }

        var table = (NativeInterop.InteropTable*)tablePtr;
        if (table->Version != NativeInterop.RML_INTEROP_TABLE_VERSION)
        {
            throw new InvalidOperationException($"Unsupported interop table version: {table->Version}");
        }

        Table = table;
    }
}