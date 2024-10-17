using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StreamParser.Common.Daybreak
{
    public interface IParser
    {
        delegate void ConnectionHandler(IConnection connection, DateTime connectionTime);
        ConnectionHandler OnNewConnection { get; set; }
        ConnectionHandler OnLostConnection { get; set; }
        void Parse(string filename);
    }
}
