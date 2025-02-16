using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public ref struct GamePacket
    {
        private readonly ReadOnlySpan<byte> _data;

        public GamePacket(ReadOnlySpan<byte> data)
        {
            _data = data;
        }

        public readonly override string ToString()
        {
            return ToString(16);
        }

        public readonly string ToString(int columns)
        {
            int rows = _data.Length / columns;
            if (_data.Length % columns != 0)
            {
                rows += 1;
            }

            int expected = (10 + columns * 4) * rows;
            var sb = new StringBuilder(expected);

            for(var i = 0; i < rows; ++i)
            {
                sb.AppendFormat("{0} |", (i * columns).ToString("X5"));

                for(var j = 0; j < columns; ++j)
                {
                    var index = (i * 16) + j;
                    if (index >= _data.Length)
                    {
                        sb.Append("   ");
                    } else
                    {
                        var c = _data[index];
                        sb.AppendFormat("{0,3}", c.ToString("X2"));
                    }
                }

                sb.Append(" | ");

                for (var j = 0; j < columns; ++j)
                {
                    var index = (i * 16) + j;
                    if (index >= _data.Length)
                    {
                        sb.Append(" ");
                    }
                    else
                    {
                        var c = _data[index];
                        var ch = (char)c;
                        if (char.IsLetterOrDigit(ch) || char.IsPunctuation(ch) || char.IsSymbol(ch) || (ch == ' '))
                        {
                            sb.Append(ch);
                        }
                        else
                        {
                            sb.Append(".");
                        }
                    }
                }

                sb.AppendLine();
            }

            return sb.ToString();
        }

        public readonly string ToModelString(int max_taken, bool hex)
        {
            int expected = Math.Min(_data.Length, max_taken) * (hex ? 2 : 1);
            if(expected <= 0)
            {
                return string.Empty;
            }

            var sb = new StringBuilder(expected);

            for(var i = 0; i < Math.Min(_data.Length, max_taken); ++i)
            {
                var c = _data[i];
                if(hex)
                {
                    sb.Append(c.ToString("X2"));
                } else
                {
                    var ch = (char)c;
                    if (char.IsLetterOrDigit(ch) || char.IsPunctuation(ch) || char.IsSymbol(ch) || (ch == ' '))
                    {
                        sb.Append(ch);
                    }
                    else
                    {
                        sb.Append(".");
                    }
                }
            }

            return sb.ToString();
        }
    }
}
