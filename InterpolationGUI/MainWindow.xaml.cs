using Microsoft.Win32;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Channels;
using System.Windows;
using System.Windows.Media.Imaging;
namespace InterpolationGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        byte[] InputBytes = { };
        int img_height = 0,img_width = 0;

        string workingDirectory = AppDomain.CurrentDomain.BaseDirectory;
        string projectDirectory = "";

        String ImgPath ="";
        public MainWindow()
        {
            InitializeComponent();

            projectDirectory = Directory.GetParent(workingDirectory)?.Parent?.Parent?.Parent?.FullName;
        }

        void SubmitValues(object s, RoutedEventArgs e)
        {
            ImgPath = InputTextBox.Text;
            MessageBox.Show(ImgPath);
            
            BitmapImage img = new(new Uri(ImgPath, UriKind.RelativeOrAbsolute));
            img_height = (int)img.Height;
            img_width = (int)img.Width;

            InputBytes = File.ReadAllBytes(ImgPath);

            InputImageDisplay.Source = img;

            MessageBox.Show($"{InputBytes.Length}");
            //IntPtr ptr = UpscaleNearestNeighbour(InputBytes, InputBytes.Length,out int outputSize);

            /*
            byte[] outputBytes = new byte[outputSize];
            Marshal.Copy(ptr, outputBytes, 0, outputSize);
            File.WriteAllBytes($"{projectDirectory}/Images/output.png", outputBytes);
            FreeMemory(ptr);
            */
        }

        void OpenDialog(object s, RoutedEventArgs e)
        {
            OpenFileDialog OFD;
            OFD = new OpenFileDialog();
            OFD.InitialDirectory = projectDirectory;

            if (OFD.ShowDialog() == true)
            {
                ImgPath = OFD.FileName;
                InputTextBox.Text = ImgPath;
            }
        }

        [DllImport("InterpolationTechniques.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr UpscaleNearestNeighbour(
            byte[] input,
            int size,
            out int outsize
        );

        [DllImport("InterpolationTechniques.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeMemory(IntPtr ptr);
    }
}