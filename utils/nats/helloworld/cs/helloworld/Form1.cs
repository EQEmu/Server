using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Eqproto;
using Google.Protobuf;

// Reference the NATS client.
using NATS.Client;

namespace helloworld
{
    public partial class Form1 : Form
    {

        // Creates a live connection to the default
        // NATS Server running locally
        IConnection c;
        ISyncSubscription sSync;

        public Form1()
        {
            InitializeComponent();
        }

        private void BtnHello_Click(object sender, EventArgs e)
        {
            if (c != null && !c.IsClosed())
            {
                // Closing a connection
                c.Close();
                TxtLog.Text += "\r\nDisconnected.";
                return;
            }

            TxtLog.Text = "Initialized";

            // Create a new connection factory to create
            // a connection.
            ConnectionFactory cf = new ConnectionFactory();

            c = cf.CreateConnection();


            TxtLog.Text += "\r\nSending hello world";
            ChannelMessage msg = new ChannelMessage();
            msg.From = "csharp";
            msg.Message = "Hello, World!";
            msg.ChanNum = 5;

            
            c.Publish("world.channel_message", msg.ToByteArray());

            // Simple synchronous subscriber
            sSync = c.SubscribeSync("world.channel_message");

            TxtLog.Text += "\r\nWaiting for message...";                        
            TmrMessage.Enabled = true;
          
        }

        private void TmrMessage_Tick(object sender, EventArgs e)
        {
            if (c == null || sSync == null)
            {
                return;
            }
            // Using a synchronous subscriber, gets the first message available,
            // waiting up to 1000 milliseconds (1 second)
            Msg m;
            try
            {
                m = sSync.NextMessage(2);                
                Application.DoEvents();
            } catch //(NATS.Client.NATSTimeoutException e)
            {
                return;
            }     
            if (m == null) return;

            ChannelMessage msg = ChannelMessage.Parser.ParseFrom(m.Data);
            TxtLog.Text += string.Format("\r\nFrom: {0} Chan_num: {1} Message: {2}", msg.From, msg.ChanNum, msg.Message);
        }
    }
}
