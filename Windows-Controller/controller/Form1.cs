using System.Diagnostics;

using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace controller
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        private readonly byte[] AuthData =
            [
            0xF0, 0x7E,
            0x4D, 0x2C, 0x59, 0x44, 0xB0, 0x59, 0x18, 0x07, 0x6A, 0x4F, 0x96, 0xD4, 0x94, 0x9B, 0xED, 0x4D,
                0x42, 0x7B, 0x63, 0x73, 0x50, 0x4D, 0x02, 0x1E, 0x69, 0x25, 0x4A, 0x70, 0x8E, 0x39, 0x1C, 0xC0,
                0x9B, 0x10, 0xB3, 0x40, 0xAC, 0xFD, 0x03, 0xA8, 0x51, 0xEA, 0xBF, 0x7A, 0x93, 0x0F, 0x40, 0x0C,
                0x69, 0x36, 0xBB, 0xA4, 0x22, 0x5A, 0x36, 0xEE, 0x9F, 0x87, 0x22, 0x8B, 0xDC, 0x65, 0x70, 0x23,
                0x84, 0x70, 0xCF, 0x18, 0x17, 0x73, 0xF3, 0x47, 0x26, 0x6C, 0xC6, 0x85, 0xFF, 0xFD, 0xEA, 0xBD,
                0x42, 0x72, 0x40, 0x3C, 0x12, 0x7C, 0x45, 0x26, 0x97, 0x8B, 0x6F, 0x68, 0x50, 0x29, 0x77, 0xA9,
                0x49, 0x46, 0x48, 0x46, 0x48, 0x48,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ];
        private static BluetoothLEAdvertisementWatcher watcher;
        private static ulong address = 0;
        private static BluetoothLEDevice BLEDevice;
        private GattDeviceService? nusService;
        private GattCharacteristic? txCharacteristic = null;
        private GattCharacteristic? rxCharacteristic = null;
        private static byte[] bAdv = new byte[24];
        private static byte[] bResponse;
        public static int PMode = 0;
        private void bConnect_Click(object sender, EventArgs e)
        {
            try
            {
                bConnect.Enabled = false;
                watcher = new BluetoothLEAdvertisementWatcher
                { ScanningMode = BluetoothLEScanningMode.Active };
                watcher.Received += Watcher_Received;
                watcher.Start();
                while (watcher.Status != BluetoothLEAdvertisementWatcherStatus.Stopped)
                { Application.DoEvents(); }
                watcher.Received -= Watcher_Received;
                ConnectAsync();
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error: {ex.Message}"); Application.Exit();
            }
        }
        private void bDisconnect_Click(object sender, EventArgs e)
        {
            try
            {
                DoDisconnect();
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error: {ex.Message}"); Application.Exit();
            }
        }
        private void bGet_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x32]; // Get PA
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtPA.Text = "";
            txtPW.Text = "";
            txtPF.Text = "";
            txtPR.Text = "";
            txtTN.Text = "";
            txtTF.Text = "";
            PMode = 9;
            Sending(data);
        }
        private void bSendPA_Click(object sender, EventArgs e)
        {
            byte step = (byte)(Convert.ToDecimal(txtPA.Text) * 10);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x32, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bSendPW_Click(object sender, EventArgs e)
        {
            byte step = (byte)(Int16.Parse(txtPW.Text) / 50);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x33, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bSendPF_Click(object sender, EventArgs e)
        {
            byte step = (byte)Int16.Parse(txtPF.Text);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x34, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bSendPR_Click(object sender, EventArgs e)
        {
            PMode = 1; //+ RampUp = step
            byte step = (byte)Int16.Parse(txtPR.Text);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x35, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bSendTN_Click(object sender, EventArgs e)
        {
            byte step = (byte)(Int16.Parse(txtTN.Text) / 10);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x37, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bSendTF_Click(object sender, EventArgs e)
        {
            byte step = (byte)(Int16.Parse(txtTF.Text) / 10);
            byte[] data = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x38, step, 0x00];
            labelTX.Text = "Set Params";
            labelRX.Text = "";
            Sending(data);
        }
        private void bGetPA_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x32]; // Get PA
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtPA.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bGetPW_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x33]; // Get PW
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtPW.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bGetPF_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x34]; // Get PF
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtPF.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bGetPR_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x35]; // Get PR
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtPR.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bGetTN_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x37]; // Get TN
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtTN.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bGetTF_Click(object sender, EventArgs e)
        {
            byte[] data = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x38]; // Get TF
            Debug.WriteLine("TX Get Params");
            labelTX.Text = "Get Params";
            labelRX.Text = "";
            txtTF.Text = "";
            PMode = 0;
            Sending(data);
        }
        private void bStimOn_Click(object sender, EventArgs e)
        {
            Debug.WriteLine("TX Stim On");
            labelTX.Text = "Stim On";
            labelRX.Text = "";
            byte[] data = [0xA5, 0x00];
            Sending(data);
        }
        private void bStimOff_Click(object sender, EventArgs e)
        {
            Debug.WriteLine("TX Stim Off");
            labelTX.Text = "Stim Off";
            labelRX.Text = "";
            byte[] data = [0xA6, 0x00];
            Sending(data);
        }
        private void Sending(byte[] data)
        {
            try
            {
                if (BLEDevice != null)
                {
                    int dataLength = data.Length;
                    const ushort polynomial = 0x1021; // CRC-16/CCITT-FALSE polynomial
                    ushort crc = 0xFFFF; // Initial value
                    foreach (byte b in data)
                    {
                        crc ^= (ushort)(b << 8); // Shift the current byte to the high bits and XOR with CRC
                        for (int i = 0; i < 8; i++) // // Perform 8 bit-shifting operations per byte
                        {
                            if ((crc & 0x8000) != 0) // If the highest bit is 1
                            { crc = (ushort)((crc << 1) ^ polynomial); }
                            else
                            { crc <<= 1; }
                        }
                    }
                    byte[] calCRC16 = [(byte)(crc & 0xFF), (byte)((crc >> 8) & 0xFF),];
                    Array.Resize(ref data, dataLength + 2);
                    Array.Copy(calCRC16, 0, data, dataLength, 2);
                    SendToRemote(data);
                }
                else
                { Debug.WriteLine("BLEDevice is null"); Application.Exit(); }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error: {ex.Message}");
            }
        }
        public async void ConnectAsync()
        {
            try
            {
                BLEDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(address);
                if (!BLEDevice.DeviceInformation.Pairing.IsPaired)
                {
                    var customPairing = BLEDevice.DeviceInformation.Pairing.Custom;
                    //var pairing = BLEDevice.DeviceInformation.Pairing.Custom;
                    customPairing.PairingRequested += (sender, args) =>
                    {
                        //Debug.WriteLine($"Pairing requested: {args.PairingKind}");
                        if (args.PairingKind == DevicePairingKinds.ProvidePin)
                        { args.Accept("000000"); }
                        else
                        { args.Accept(); }
                    };
                    var result = await customPairing.PairAsync(DevicePairingKinds.ProvidePin);
                    if (result.Status != DevicePairingResultStatus.Paired)
                    {
                        Debug.WriteLine($"Pairing failed: {result.Status}");
                        DoDisconnect();
                        return;
                    }
                }
                if (BLEDevice.ConnectionStatus != BluetoothConnectionStatus.Connected)
                {
                    Debug.WriteLine("Failed to connect");
                    DoDisconnect();
                    return;
                }
                var gattServicesResult = await BLEDevice.GetGattServicesAsync();
                nusService = null;
                if (gattServicesResult.Status == GattCommunicationStatus.Success)
                {
                    //Debug.WriteLine($"Found {gattServicesResult.Services.Count} GATT services.");
                    foreach (var service in gattServicesResult.Services)
                    {
                        if (service.Uuid == Guid.Parse("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")) // UUID
                        {
                            nusService = service;
                            //Debug.WriteLine($"Found target service: {service.Uuid}");
                            var characteristicsResult = await nusService.GetCharacteristicsAsync();
                            txCharacteristic = null;
                            rxCharacteristic = null;
                            if (characteristicsResult.Status != GattCommunicationStatus.Success)
                            {
                                Debug.WriteLine("Failed to retrieve characteristics.");
                                DoDisconnect();
                                return;
                            }
                            foreach (var characteristic in characteristicsResult.Characteristics)
                            {
                                if (characteristic.Uuid == Guid.Parse("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")) // TX UUID
                                {
                                    //Debug.WriteLine("TX Characteristic found.");
                                    var status = await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                                        GattClientCharacteristicConfigurationDescriptorValue.None);
                                    //Debug.WriteLine($"TXNone:{status}");
                                    if (status == GattCommunicationStatus.Success)
                                    {
                                        status = await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                                            GattClientCharacteristicConfigurationDescriptorValue.Notify);
                                        //Debug.WriteLine($"TXNotify:{status}");
                                        if (status == GattCommunicationStatus.Success)
                                        {
                                            txCharacteristic = characteristic;
                                            txCharacteristic.ValueChanged += SendFromRemote;
                                            //Debug.WriteLine("TX added");
                                        }
                                        else
                                        {Debug.WriteLine($"Failed to enable TX: {status}"); DoDisconnect(); }
                                    }
                                    else
                                    {Debug.WriteLine($"Failed to disable TX: {status}"); DoDisconnect(); }
                                }
                                else if (characteristic.Uuid == Guid.Parse("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")) // RX UUID
                                {
                                    rxCharacteristic = characteristic;
                                    //Debug.WriteLine("RX Characteristic found.");
                                }
                            }
                        }
                    }
                    if (nusService == null || txCharacteristic == null || rxCharacteristic == null)
                    {
                        DoDisconnect();
                        Debug.WriteLine("Connection failed");
                    }
                    else
                    {
                        bConnect.BeginInvoke(((Action)(() => bConnect.Enabled = false)));
                        bDisconnect.BeginInvoke(((Action)(() => bDisconnect.Enabled = true)));
                        bGet.BeginInvoke(((Action)(() => bGet.Enabled = true)));
                        bSendPA.BeginInvoke(((Action)(() => bSendPA.Enabled = true)));
                        bSendPW.BeginInvoke(((Action)(() => bSendPW.Enabled = true)));
                        bSendPF.BeginInvoke(((Action)(() => bSendPF.Enabled = true)));
                        bSendPR.BeginInvoke(((Action)(() => bSendPR.Enabled = true)));
                        bSendTN.BeginInvoke(((Action)(() => bSendTN.Enabled = true)));
                        bSendTF.BeginInvoke(((Action)(() => bSendTF.Enabled = true)));
                        bStimOn.BeginInvoke(((Action)(() => bStimOn.Enabled = true)));
                        bStimOff.BeginInvoke(((Action)(() => bStimOff.Enabled = false)));
                        bGetPA.BeginInvoke(((Action)(() => bGetPA.Enabled = true)));
                        bGetPW.BeginInvoke(((Action)(() => bGetPW.Enabled = true)));
                        bGetPF.BeginInvoke(((Action)(() => bGetPF.Enabled = true)));
                        bGetPR.BeginInvoke(((Action)(() => bGetPR.Enabled = true)));
                        bGetTN.BeginInvoke(((Action)(() => bGetTN.Enabled = true)));
                        bGetTF.BeginInvoke(((Action)(() => bGetTF.Enabled = true)));
                        Debug.WriteLine("TX Admin Mode");
                        labelTX.Text = "Admin Mode";
                        labelRX.Text = "";
                        Sending(AuthData);
                    }
                }
                else
                {
                    Debug.WriteLine("Failed to get GATT services");
                    DoDisconnect();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error: {ex.Message}");
            }
        }
        private void Watcher_Received(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
        {
            BluetoothLEAdvertisement adv = args.Advertisement;
            if (adv.ManufacturerData.Count >= 1)
            {
                foreach (BluetoothLEManufacturerData md in adv.ManufacturerData)
                {
                    if (md.CompanyId == 0xFFFF || md.CompanyId == 0xF0F0)
                    {
                        address = args.BluetoothAddress;
                        DataReader reader = DataReader.FromBuffer(md.Data);
                        reader.ReadBytes(bAdv);
                        if (md.CompanyId == 0xFFFF) ///DVT
                        { bAdv[23] = 1; }
                        if (md.CompanyId == 0xF0F0) ///SRS 
                        { bAdv[23] = 2; }
                        watcher.Stop();
                        break;
                    }
                }
            }
        }
        private void SendFromRemote(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            var reader = DataReader.FromBuffer(args.CharacteristicValue);
            Array.Resize(ref bResponse, (int)reader.UnconsumedBufferLength);
            reader.ReadBytes(bResponse);
            switch (bResponse[0])
            {
                case 0xA5: //Stim On
                    bStimOn.BeginInvoke(((Action)(() => bStimOn.Enabled = false)));
                    bStimOff.BeginInvoke(((Action)(() => bStimOff.Enabled = true)));
                    Debug.WriteLine("RX Stim On");
                    labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Stim On")));
                    break;
                case 0xA6: //Stim Off
                    bStimOn.BeginInvoke(((Action)(() => bStimOn.Enabled = true)));
                    bStimOff.BeginInvoke(((Action)(() => bStimOff.Enabled = false)));
                    Debug.WriteLine("RX Stim Off");
                    labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Stim Off")));
                    break;
                case 0xAC: //Get params
                    switch (bResponse[6])
                    {
                        case 0x32: // Get PA
                            txtPA.BeginInvoke(((Action)(() => txtPA.Text = (Math.Round((((int)bResponse[8] * 256 + (int)bResponse[7]) * 0.1),1)).ToString())));
                            if (PMode == 9)
                            {
                                byte[] data3 = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x33];
                                Sending(data3);
                            }
                            else
                            {
                                Debug.WriteLine("RX Get Params");
                                labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            }
                            break;
                        case 0x33: // Get PW
                            txtPW.BeginInvoke(((Action)(() => txtPW.Text = (((int)bResponse[8] * 256 + (int)bResponse[7]) * 50).ToString())));
                            if (PMode == 9)
                            {
                                byte[] data4 = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x34];
                                Sending(data4);
                            }
                            else
                            {
                                Debug.WriteLine("RX Get Params");
                                labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            }
                            break;
                        case 0x34: // Get PF
                            txtPF.BeginInvoke(((Action)(() => txtPF.Text = ((int)bResponse[8] * 256 + (int)bResponse[7]).ToString())));
                            if (PMode == 9)
                            {
                                byte[] data5 = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x35];
                                Sending(data5);
                            }
                            else
                            {
                                Debug.WriteLine("RX Get Params");
                                labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            }
                            break;
                        case 0x35: // Get PR
                            txtPR.BeginInvoke(((Action)(() => txtPR.Text = ((int)bResponse[8] * 256 + (int)bResponse[7]).ToString())));
                            if (PMode == 9)
                            {
                                byte[] data7 = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x37];
                                Sending(data7);
                            }
                            else
                            {
                                Debug.WriteLine("RX Get Params");
                                labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            }
                            break;
                        case 0x37: // Get TN
                            txtTN.BeginInvoke(((Action)(() => txtTN.Text = (((int)bResponse[8] * 256 + (int)bResponse[7]) * 10).ToString())));
                            if (PMode == 9)
                            {
                                byte[] data8 = [0xAC, 0x04, 0x53, 0x50, 0x30, 0x38];
                                Sending(data8);
                            }
                            else
                            {
                                Debug.WriteLine("RX Get Params");
                                labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            }
                            break;
                        case 0x38: // Get TF
                            txtTF.BeginInvoke(((Action)(() => txtTF.Text = (((int)bResponse[8] * 256 + (int)bResponse[7]) * 10).ToString())));
                            Debug.WriteLine("RX Get Params");
                            labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Get Params")));
                            break;
                        default:
                            break;
                    }
                    break;
                case 0xAD: //Set params 
                    Debug.WriteLine("RX Set Params");
                    labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Set Params")));
                    if (PMode == 1)
                    {
                        PMode = 0; //+ RampDown = step
                        byte step = (byte)Int16.Parse(txtPR.Text);
                        if (step > 0)
                        {
                            byte[] data1 = [0xAD, 0x06, 0x53, 0x50, 0x30, 0x36, step, 0x00];
                            Sending(data1);
                        }
                    };
                    break;
                case 0xF0: //Admin mode
                    Debug.WriteLine("RX Admin Mode");
                    Debug.WriteLine("TX No whitelist");
                    labelRX.BeginInvoke(((Action)(() => labelRX.Text = "Admin Mode")));
                    labelTX.BeginInvoke(((Action)(() => labelTX.Text = "No whitelist")));
                    byte[] dataf2 = [0xF2, 0x07, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00]; // Disable whitelist
                    Sending(dataf2);
                    break;
                case 0xF2: //Disable whitelist
                    Debug.WriteLine("RX No whitelist");
                    labelRX.BeginInvoke(((Action)(() => labelRX.Text = "No whitelist")));
                    labelTX.BeginInvoke(((Action)(() => labelTX.Text = "Set MaxA")));
                    Debug.WriteLine("TX Set MaxA");
                    byte[] data10 = [0xAD, 0x06, 0x53, 0x50, 0x31, 0x31, 50, 0x00];
                    Sending(data10);
                    break;
                default:
                    break;
            }
        }
        public async void SendToRemote(byte[] data)
        {
            if (rxCharacteristic != null)
            {
                try
                {
                    var writer = new DataWriter();
                    writer.WriteBytes(data);
                    var status = await rxCharacteristic.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithResponse);
                    if (status != GattCommunicationStatus.Success)
                    { //Debug.WriteLine($"1.TX {Convert.ToHexString(data)}"); 
                        Debug.WriteLine("TX failed"); Application.Exit(); }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine($"SendToRemote Error {ex.Message}"); Application.Exit();
                }
            }
        }
        public async void DoDisconnect()
        {
            if (BLEDevice != null)
            {
                try
                {
                    if (BLEDevice.DeviceInformation.Pairing.IsPaired)
                    {
                        DeviceUnpairingResult dupr = await BLEDevice.DeviceInformation.Pairing.UnpairAsync();
                        while (dupr.Status != DeviceUnpairingResultStatus.Unpaired)
                        { Application.DoEvents(); }
                    }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine($"Error during unpairing: {ex.Message}");
                }
                nusService?.Dispose();
                nusService = null;
                txCharacteristic = null;
                rxCharacteristic = null;
                BLEDevice?.Dispose();
                BLEDevice = null;
                Application.Exit();
            }
            try
            {
                bConnect.BeginInvoke(((Action)(() => bConnect.Enabled = true)));
                bDisconnect.BeginInvoke(((Action)(() => bDisconnect.Enabled = false)));
                bGet.BeginInvoke(((Action)(() => bGet.Enabled = false)));
                bSendPA.BeginInvoke(((Action)(() => bSendPA.Enabled = false)));
                bSendPW.BeginInvoke(((Action)(() => bSendPW.Enabled = false)));
                bSendPF.BeginInvoke(((Action)(() => bSendPF.Enabled = false)));
                bSendPR.BeginInvoke(((Action)(() => bSendPR.Enabled = false)));
                bSendTN.BeginInvoke(((Action)(() => bSendTN.Enabled = false)));
                bSendTF.BeginInvoke(((Action)(() => bSendTF.Enabled = false)));
                bStimOn.BeginInvoke(((Action)(() => bStimOn.Enabled = false)));
                bStimOff.BeginInvoke(((Action)(() => bStimOff.Enabled = false)));
                bGetPA.BeginInvoke(((Action)(() => bGetPA.Enabled = false)));
                bGetPW.BeginInvoke(((Action)(() => bGetPW.Enabled = false)));
                bGetPF.BeginInvoke(((Action)(() => bGetPF.Enabled = false)));
                bGetPR.BeginInvoke(((Action)(() => bGetPR.Enabled = false)));
                bGetTN.BeginInvoke(((Action)(() => bGetTN.Enabled = false)));
                bGetTF.BeginInvoke(((Action)(() => bGetTF.Enabled = false)));
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Button Error: {ex.Message}"); 
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {

        }
    }
}
