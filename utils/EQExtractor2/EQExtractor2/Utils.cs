//
// The BitStream class is a direct C# translation of the class of the same name in the ShowEQ source, so the following
// Copyright notice applies to that:
//
// ShowEQ Distributed under GPL
// http://www.sourceforge.net/projects/seq
//
//  Copyright 2004 Zaphod (dohpaz@users.sourceforge.net).
//
// -----------------------------------------------------------
//
// All other code:
//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//


using System;
using System.Text;

namespace MyUtils
{
    public class ByteStream
    {
        public ByteStream(byte[] NewBuffer)
        {
            Buffer = NewBuffer;

            BufferPointer = 0;
        }

        public uint Length()
        {
            return (uint)Buffer.Length;
        }

        public UInt32 ReadUInt32()
        {
            UInt32 Value = BitConverter.ToUInt32(Buffer, BufferPointer);

            BufferPointer += 4;

            return Value;
        }

        public Int32 ReadInt32()
        {
            Int32 Value = BitConverter.ToInt32(Buffer, BufferPointer);

            BufferPointer += 4;

            return Value;
        }

        public float ReadSingle()
        {
            float Value = BitConverter.ToSingle(Buffer, BufferPointer);

            BufferPointer += 4;

            return Value;
        }
        public UInt16 ReadUInt16()
        {
            UInt16 Value = BitConverter.ToUInt16(Buffer, BufferPointer);

            BufferPointer += 2;

            return Value;
        }

        public Int16 ReadInt16()
        {
            Int16 Value = BitConverter.ToInt16(Buffer, BufferPointer);

            BufferPointer += 2;

            return Value;
        }
        public byte ReadByte()
        {
            return Buffer[BufferPointer++];
        }

        public byte[] ReadBytes(int Count)
        {
            byte[] Slice = new byte[Count];

            Array.Copy(Buffer, BufferPointer, Slice, 0, Count);

            BufferPointer += Count;

            return Slice;
        }

        public void SkipBytes(int Count)
        {
            BufferPointer += Count;
        }

        public string ReadString(bool Escape)
        {
            string Result = "";

            while(Buffer[BufferPointer] != 0)
            {
                if (Escape && ((char)Buffer[BufferPointer] == '\''))
                    Result += '\\';

                Result += (char)Buffer[BufferPointer++];
            }

            ++BufferPointer;

            return Result;
        }

        public string ReadFixedLengthString(int Length, bool Escape)
        {
            int StartingPosition = BufferPointer;

            string Result = "";

            while (Buffer[BufferPointer] != 0)
            {
                if (Escape && ((char)Buffer[BufferPointer] == '\''))
                    Result += '\\';

                Result += (char)Buffer[BufferPointer++];
            }

            BufferPointer = StartingPosition + Length;

            return Result;
        }

        public void SetPosition(int NewPosition)
        {
            BufferPointer = NewPosition;
        }

        public int GetPosition()
        {
            return BufferPointer;
        }

        public byte[] Buffer;

        int BufferPointer;
    }

    class BitStream
    {
        public BitStream(byte[] Data, UInt32 Length)
        {
            m_data = Data;
            TotalBits = Length * 8;
            Reset();
        }
        public void Reset()
        {
            CurrentBit = 0;
        }
        public UInt32 readUInt(int bitCount)
        {
            // Make sure we have the bits first.
            if (CurrentBit + bitCount > TotalBits)
            {
                return 0;
            }

            UInt32 currentByte = (CurrentBit >> 3);
            UInt32 Out = 0;

            // Partial bytes in the lead and end. Full bytes in the middle.
            Int32 leadPartialBitCount = 8 - ((Int32)CurrentBit % 8);
            UInt32 middleByteCount;
            Int32 tailPartialBitCount;

            if (leadPartialBitCount == 8)
            {
                // Lead partial is a byte. So just put it in the middle.
                leadPartialBitCount = 0;
            }

            if (leadPartialBitCount > bitCount)
            {
                // All the bits we need are in the lead partial. Note that when
                // the lead partial is byte aligned, this won't process it. Instead
                // it will be handled by the tailPartialBitCount.
                Out = m_data[currentByte] & (((UInt32)1 << leadPartialBitCount) - 1);
                CurrentBit += (UInt32)bitCount;

                return Out >> (leadPartialBitCount - bitCount);
            }
            else
            {
                // Spanning multiple bytes. leadPartialBitCount is correct.
                // Calculate middle and tail.
                middleByteCount = ((UInt32)bitCount - (UInt32)leadPartialBitCount) / 8;
                tailPartialBitCount =
                    bitCount - ((Int32)leadPartialBitCount + (Int32)middleByteCount*8);
            }

            if (leadPartialBitCount > 0)
            {
                // Pull in partial from the lead byte
                Out |= m_data[currentByte] & (UInt32)((1 << leadPartialBitCount) - 1);
                currentByte++;
            }

            // Middle
            for (int i=0; i<middleByteCount; i++)
            {
                Out = (Out << 8) | m_data[currentByte];
                currentByte++;
            }

            // And the end.
            if (tailPartialBitCount > 0)
            {
                Out = (UInt32)((Out << tailPartialBitCount) |
                (m_data[currentByte] >> (8 - tailPartialBitCount)));
            }

            // Update the current bit
            CurrentBit += (UInt32)bitCount;

            return Out;
        }

        public Int32 readInt(Int32 bitCount)
        {
            // Sign
            UInt32 sign = readUInt(1);
            Int32 retval = (Int32)readUInt(bitCount - 1);

            return retval * ((sign == 1) ? -1 : 1);
        }


        private byte[] m_data;
        private UInt32 TotalBits;
        private UInt32 CurrentBit;

    }
    class Utils
    {
        public static string HexDump(byte[] bytes)
        {
            StringBuilder Dump = new StringBuilder();

            string Hex = "", Ascii = "";

            int Offset = 0, i = 0, Length = bytes.Length;

            while (i < Length)
            {
                Hex += bytes[i].ToString("x2") + " ";

                Ascii += (((bytes[i] >= 32) && (bytes[i] <= 126)) ? ((char)bytes[i]).ToString() : ".");

                if (((++i % 16) == 0) || (i == Length))
                {
                    Dump.AppendFormat("{0:000} | {1} | {2}\r\n", Offset, Hex.PadRight(48), Ascii);

                    Hex = Ascii = "";

                    Offset = i;
                }
            }
            return Dump.ToString();
        }

        public static string ReadNullTerminatedString(byte[] Buffer, int Offset, int MaxSize, bool Escape)
        {
            string Result = "";

            for (int i = 0; i < MaxSize && Buffer[Offset + i] != 0; ++i)
            {
                if(Escape && ((char)Buffer[Offset + i] == '\''))
                    Result += '\\';

                Result += (char)Buffer[Offset + i];
            }

            return Result;
        }

        public static string MakeCleanName(string Name)
        {
            StringBuilder CleanName = new StringBuilder();

            foreach(char c in Name)
                if(!Char.IsDigit(c))
                    CleanName.Append(c);

            return CleanName.ToString();
        }

        public static float EQ19ToFloat(Int32 EQ19Value)
        {
            if ((EQ19Value & 0x40000) > 0)
                EQ19Value = -(0x7FFFF - EQ19Value + 1);

            return (float)EQ19Value / (float)(1 << 3);
        }


    }


}
