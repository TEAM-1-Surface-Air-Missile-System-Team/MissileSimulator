using nframework.nom;
using NOMHandlerLib.Concrete;
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
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Windows.Threading;
using System.Runtime.Remoting.Messaging;

namespace MiniProject_GUI
{
    using NOMHandlerLib.Core;
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MainWindow : Window
    {
        
        private readonly NOMHandler nomHandler = null;

        private readonly Dictionary<string, HashSet<uint>> instanceIDByName = new Dictionary<string, HashSet<uint>>();
        private readonly Dictionary<string, HashSet<uint>> registeredInstanceID = new Dictionary<string, HashSet<uint>>();
        private readonly Dictionary<uint, NOM> discoveredNOMInstance = new Dictionary<uint, NOM>();
        private readonly Dictionary<uint, NOM> registeredNOMInstance = new Dictionary<uint, NOM>();

        private static Mutex mutObj = new Mutex();
        private static Mutex mutIntr = new Mutex();

        public MainWindow()
        {
            List<string> nomFilePaths = new List<string>();
            nomFilePaths.Add(Environment.CurrentDirectory + @"\UIManager\UIManager.xml");
            nomFilePaths.Add(Environment.CurrentDirectory + @"\NewRoleManager\NewRoleManager.xml");

            // NOMHandler 객체 생성.
            nomHandler = new NOMHandler(nomFilePaths);

            // NOM 메시지 수신 이벤트 구독.
            nomHandler.MessageReceived += OnNOMMessageReceived;

            // NOM 메시지 송신 이벤트 구독.
            nomHandler.MessageSent += OnNOMMessageSent;

            // 메인 윈도우 Loaded 이벤트 구독.
            Loaded += MainWindow_Loaded;

            // 메인 윈도우 Closed 이벤트 구독.
            Closed += MainWindow_Closed;

            InitializeComponent();


        }

        /// <summary>
        /// 메인 윈도우 Loaded 이벤트 구독 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateButton.IsEnabled = false;
            RegisterButton.IsEnabled = false;
            DeleteButton.IsEnabled = false;
            SendEventReport.IsEnabled = false;
            // NOMHandler의 윈도우 메시지 처리 함수 등록.
            (PresentationSource.FromVisual(this) as HwndSource)?.AddHook(nomHandler.WndProcCallback);

            // NOM 메시지 전시
            MessageTable.ItemsSource = nomHandler.NOMMessageList;

            PlugInButton.IsEnabled = false;
            PlugOutButton.IsEnabled = false;

            // C++ NFConnect 객체 생성.
            if (nomHandler.CreateNFConnect())
            {

                // 플러그 인.
                nomHandler.DoPlugIn();

                // 윈도우 핸들 등록
                nomHandler.SetWindowHandle(new WindowInteropHelper(this).Handle);

                // 런 스타트.
                nomHandler.ExecStart();

                PlugOutButton.IsEnabled = true;
                SendEventReport.IsEnabled = true;

            }
            else
            {
                PlugInButton.IsEnabled = true;
                MessageBox.Show("C++ NFConnect 객체를 생성하지 못하였습니다.");
            }


        }

        /// <summary>
        /// 메인 윈도우 Closed 이벤트 구독 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainWindow_Closed(object sender, EventArgs e)
        {
            nomHandler.ExecStop();

            // 플러그 아웃.
            nomHandler.DoPlugOut();

            // C++ NFConnect 객체 소멸.
            nomHandler.DestroyNFConnect();

            // 프로세스 종료.
            System.Diagnostics.Process.GetCurrentProcess().Kill();
            Application.Current.Shutdown();
            Environment.Exit(0);
        }

        /// <summary>
        /// NOM 메시지 송신 이벤트 구독 메소드.
        /// </summary>
        /// <param name="nomInstance"></param>
        /// <param name="operation"></param>
        private void OnNOMMessageSent(NOM nomInstance, EMessageOperation operation)
        {
            // TODO : NOM 메시지 송신 처리.
        }

        /// <summary>
        /// NOM 메시지 수신 이벤트 구독 메소드.
        /// </summary>
        /// <param name="nomInstance"></param>
        /// <param name="operation"></param>
        private void OnNOMMessageReceived(NOM nomInstance, EMessageOperation operation)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Send, new Action((delegate
            {
                mutObj.WaitOne();
                switch (operation)
                {
                    case EMessageOperation.Reflect:
                        {
                            NOM innerNom = nomInstance;

                            Console.WriteLine("[OnNOMMessageReceived()] C# reflect object class {" + innerNom.name + "}.");

                            // do something

                            break;
                        }
                    case EMessageOperation.Receive:
                        {
                            NOM innerNom = nomInstance.clone();

                            Console.WriteLine("[OnNOMMessageReceived()] C# receive interaction class {" + innerNom.name + "}.");

                            // do something

                            break;
                        }
                    case EMessageOperation.Discover:
                        {
                            if (!discoveredNOMInstance.ContainsKey(nomInstance.instanceID)) // if not exist, discover
                            {
                                discoveredNOMInstance[nomInstance.instanceID] = nomInstance;
                                NOM innerNom = discoveredNOMInstance[nomInstance.instanceID];

                                Console.WriteLine("[OnNOMMessageReceived()] C# discover object class {" + innerNom.name + "}.");

                                // do something

                                #region GUI
                                // start inserting GUI data
                                if (!instanceIDByName.ContainsKey(innerNom.name))
                                {
                                    // newly add HashSet and insert istanceID to HashSet
                                    instanceIDByName.Add(innerNom.name, new HashSet<uint>() { innerNom.instanceID });
                                }
                                else
                                {
                                    // newly insert instanceID to HashSet
                                    instanceIDByName[innerNom.name].Add(innerNom.instanceID);
                                }
                                // end inserting GUI data
                                
                                // start updating GUI
                                MessageTable.SelectedItem = nomHandler.GetNMessage(innerNom.msgID);
                                InstanceList.ItemsSource = null;
                                InstanceList.ItemsSource = instanceIDByName[innerNom.name];
                                // end updating GUI
                                #endregion

                            }
                            else
                            {
                                // already discovered
                            }
                            break;
                        }
                    case EMessageOperation.Remove:
                        {
                            NOM innerNom = discoveredNOMInstance[nomInstance.instanceID];
                            string nomName = innerNom.name;

                            // do something

                            #region GUI
                            // start removing GUI data
                            if (instanceIDByName[innerNom.name].Contains(innerNom.instanceID))
                            { // remove
                                instanceIDByName[innerNom.name].Remove(innerNom.instanceID);
                                // start updating GUI
                                MessageTable.SelectedItem = nomHandler.GetNMessage(innerNom.msgID);
                                InstanceList.ItemsSource = null;
                                InstanceList.ItemsSource = instanceIDByName[innerNom.name];
                                // end updating GUI
                            }
                            // end removing GUI data
                            #endregion
                            innerNom = null;

                            discoveredNOMInstance.Remove(nomInstance.instanceID);

                            Console.WriteLine("[OnNOMMessageReceived()] C# remove object class {" + nomName + "}.");
                            break;
                        }
                    default:
                        break;
                }

                mutObj.ReleaseMutex();
            })));
        }

        /// <summary>
        /// 메시지 목록 테이블 선택 변경 이벤트 구독 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MessageTable_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (MessageTable.SelectedItem is NMessage nMessage && !PlugInButton.IsEnabled)
            {
                

                if (instanceIDByName.ContainsKey(nMessage.name))
                {
                    InstanceList.ItemsSource = instanceIDByName[nMessage.name];
                }
                else
                {
                    InstanceList.ItemsSource = null;
                }

                if (nMessage.name.StartsWith("GUI_") && !PlugInButton.IsEnabled)
                {
                    RegisterButton.IsEnabled = false;
                    DeleteButton.IsEnabled = false;
                    UpdateButton.IsEnabled = false;
                }

                else if (nMessage.name.EndsWith("_GUI") && !PlugInButton.IsEnabled)
                {

                    RegisterButton.IsEnabled = false;
                    DeleteButton.IsEnabled = false;
                    UpdateButton.IsEnabled = false;

                }
                else if (nMessage.type == ENOMType.OBJECT && !PlugInButton.IsEnabled)
                {

                    RegisterButton.IsEnabled = true;
                    DeleteButton.IsEnabled = true;
                    UpdateButton.IsEnabled = true;

                }
                else
                {
                    RegisterButton.IsEnabled = false;
                    DeleteButton.IsEnabled = false;
                    UpdateButton.IsEnabled = false;
                }

            }

            
 
        }

        /// <summary>
        ///  PlugIn 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PlugInButton_Click(object sender, RoutedEventArgs e)
        {
            PlugInButton.IsEnabled = false;
            // C++ NFConnect 객체 생성.
            if (nomHandler.CreateNFConnect())
            {

                // 플러그 인.
                nomHandler.DoPlugIn();

                // 윈도우 핸들 등록
                nomHandler.SetWindowHandle(new WindowInteropHelper(this).Handle);

                // exec 스타트.
                nomHandler.ExecStart();

                PlugOutButton.IsEnabled = true;
                SendEventReport.IsEnabled = true;

            }
            else
            {
                PlugInButton.IsEnabled = true;
                MessageBox.Show("C++ NFConnect 객체를 생성하지 못하였습니다.");
            }
        }


        /// <summary>
        ///  PlugOut 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PlugOutButton_Click(object sender, RoutedEventArgs e)
        {
            bool dirty = false;
            foreach (var pair in registeredInstanceID)
            {
                if (pair.Value != null && 0 < pair.Value.Count )
                {
                    var instanceIDSet = pair.Value;
                    while (0 < instanceIDSet.Count)
                    {
                        var firstElem = instanceIDSet.First();
                        nomHandler.DeleteNOMMessage(firstElem);
                        instanceIDSet.Remove(firstElem);
                        dirty = true;
                    }
                }
            }
            foreach (var pair in instanceIDByName)
            {
                if (pair.Value != null && 0 < pair.Value.Count)
                {
                    pair.Value.Clear();
                    dirty = true;
                }
            }
            registeredNOMInstance.Clear();
            InstanceList.ItemsSource = null;

            if (dirty)
            {
                MessageBox.Show("Register된 메시지를 정리했습니다. Plug Out을 다시 클릭해주세요.");
                return;
            }
            else
            {
                PlugOutButton.IsEnabled = false;
                // C++ NFConnect 객체 생성.
                if (nomHandler != null)
                {
                    InstanceList.ItemsSource = null;
                    // exec 스탑.
                    nomHandler.ExecStop();

                    // 플러그 아웃.
                    nomHandler.DoPlugOut();

                    PlugOutButton.IsEnabled = false;
                    PlugInButton.IsEnabled = true;
                    RegisterButton.IsEnabled = false;
                    DeleteButton.IsEnabled = false;
                    UpdateButton.IsEnabled = false;
                    SendEventReport.IsEnabled = false;
                }
                else
                {
                    PlugInButton.IsEnabled = true;
                }
            }
        }

        /// <summary>
        /// Register 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RegisterButton_Click(object sender, RoutedEventArgs e)
        {
            if (MessageTable.SelectedItem == null)
            {
                MessageBox.Show("선택된 메시지가 없습니다.");
            }
            else
            {
                if (MessageTable.SelectedItem is NMessage nMessage)
                {
                    if (nMessage.type == ENOMType.OBJECT)
                    {
                        if (nMessage.sharing == ESharing.PUBLISHSUBSCRIBE || nMessage.sharing == ESharing.PUBLISH)
                        {
                            registerToManager(nMessage.name);
                        }
                        else
                        {
                            MessageBox.Show("수신 전용 메시지(SUBSCRIBE)는 Register할 수 없습니다.");
                        }
                    }
                    else
                    {
                        MessageBox.Show("Interaction 메시지는 Register할 수 없습니다.");
                    }
                }
            }
        }

        private void registerToManager (string msgName)
        {
            uint instanceID = nomHandler.RegisterNOMMessage(msgName);

            if (instanceIDByName.ContainsKey(msgName))
                instanceIDByName[msgName].Add(instanceID);
            else
                instanceIDByName.Add(msgName, new HashSet<uint>() { instanceID });

            if (registeredInstanceID.ContainsKey(msgName))
                registeredInstanceID[msgName].Add(instanceID);
            else
                registeredInstanceID.Add(msgName, new HashSet<uint>() { instanceID });

            if (!registeredNOMInstance.ContainsKey(instanceID))
                registeredNOMInstance.Add(instanceID, nomHandler.GetRegisteredNOMInstance(instanceID));
            else 
            { }

            InstanceList.ItemsSource = null;
            InstanceList.ItemsSource = instanceIDByName[msgName];
        }

        /// <summary>
        /// Delete 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void DeleteButton_Click(object sender, RoutedEventArgs e)
        {
            if (MessageTable.SelectedItem is NMessage nMessage)
            {
                if (nMessage.type == ENOMType.OBJECT)
                {
                    if (InstanceList.SelectedItem is uint instanceID)
                    {
                        registeredNOMInstance.Remove(instanceID);
                        nomHandler.DeleteNOMMessage(instanceID);
                        if (instanceIDByName.ContainsKey(nMessage.name) && registeredInstanceID.ContainsKey(nMessage.name))
                        {
                            instanceIDByName[nMessage.name].Remove(instanceID);
                            registeredInstanceID[nMessage.name].Remove(instanceID);


                            InstanceList.ItemsSource = null;
                            InstanceList.ItemsSource = instanceIDByName[nMessage.name];
                        }
                    }
                    else
                    {
                        MessageBox.Show("선택된 인스턴스 ID가 없습니다.");
                    }
                }
                else
                {
                    MessageBox.Show("Interaction 메시지는 Delete할 수 없습니다.");
                }
            }
        }

        /// <summary>
        /// SendEventReport 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SendEventReportButton_Click(object sender, RoutedEventArgs e)
        {
            var nomInstance = default(NOM);

            // Interaction 메시지 전송.
            nomInstance = nomHandler.GetNMessage("EventReport")?.createNOMInstance();

            if (nomInstance != null)
            {
                // TODO : nomInstance 필드값 설정.
                try
                {
                    nomInstance.setValue("OriginatingEntity.FederateIdentifier.SiteID", new NUShort(100));
                    nomHandler.SendNOMMessage(nomInstance);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.ToString());
                }
            }
 
        }

        /// <summary>
        /// Update 버튼 클릭 이벤트 메소드.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UpdateButton_Click(object sender, RoutedEventArgs e)
        {
            var nomInstance = default(NOM);
            if (MessageTable.SelectedItem is NMessage nMessage)
            {
                if (nMessage.type == ENOMType.OBJECT)
                {
                    if (InstanceList.SelectedItem is uint instanceID)
                    {
                        // OBJECT 메시지 전송.
                        nomInstance = nomHandler.GetRegisteredNOMInstance(instanceID);
                        if (nomInstance != null)
                        {
                            // TODO : nomInstance 필드값 설정.
                           
                        }

                    }
                    else
                    {
                        MessageBox.Show("선택된 인스턴스 ID가 없습니다.");
                    }
                }
                else
                {

                }
            }

            if (nomInstance != null)
            {
                nomHandler.SendNOMMessage(nomInstance);
            }
        }
    }
}
