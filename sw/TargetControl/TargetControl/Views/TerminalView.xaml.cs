using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TargetControl
{
    /// <summary>
    /// Interaction logic for terminal.xaml
    /// </summary>
    public partial class TerminalView : UserControl
    {
        public TerminalView()
        {
            InitializeComponent();
        }
        private void UpdateTerminalOutput(string text, Brush color)
        {
            TextPointer start = terminalOutputText.Document.ContentStart;
            terminalOutputText.AppendText(text);
            TextPointer end = terminalOutputText.Document.ContentEnd;
            terminalOutputText.Selection.Select(start, end);
            terminalOutputText.Selection.ApplyPropertyValue(RichTextBox.ForegroundProperty, color);
            terminalOutputText.ScrollToEnd();
        }
        private void terminalInputText_KeyUp(object sender,KeyEventArgs e)
        {
            if (e.Key==Key.Enter)
            {
                string input_data = terminalInputText.Text + Environment.NewLine;
                UpdateTerminalOutput(input_data,Brushes.Red);
                terminalInputText.Clear();
            }
        }
    }
}
