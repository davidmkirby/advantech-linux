package Counter_BufferedPulseWidthMeasurement;

import java.awt.Component;
import java.awt.EventQueue;
import java.awt.SystemColor;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JList;
import javax.swing.ListCellRenderer;
import javax.swing.ListSelectionModel;
import javax.swing.border.Border;
import javax.swing.border.LineBorder;
import javax.swing.SwingConstants;
import javax.swing.JButton;

import Automation.BDaq.*;
import Common.Global;

import org.eclipse.wb.swing.FocusTraversalOnArray;

import java.awt.Font;

import javax.swing.JComboBox;
import javax.swing.JTextArea;

public class BufferedPulseWidthMeasurement extends JFrame implements ActionListener{
	// define the serialization number.
	private static final long serialVersionUID = 1L;
	
	private JPanel contentPane;
	private JTextField txtChannelStatus;
	private JList lstCounterValue;
	private JComboBox cbxDisplayedChannel;
	private DefaultListModel model = new DefaultListModel();
	private JButton btnStart;
	private JButton btnStop;
	private JButton btnConfigure;
	private PulseWidth[] dataBuf;
	private PulseWidth[][] chs_dataBuf;
	private String[] ch_status;
	private int eventChannel;
	private int displayChannel;
	private boolean [] channelEnable;
	private int sectionLength;
	private int sectionCount;
	private int samplesCount;
	private boolean isRunning;
	private boolean haveData;
	private int listCount;
	
	private final int MAX_ROW_COUNT = 10;
	
	ConfigureParameter configure = new ConfigureParameter();
	private ConfigureDialog configureDialog;
	private BufferedPwMeterCtrl bfdPwMeterCtrl = new BufferedPwMeterCtrl(); 
	private int countOfValue = 0;	
	private int currentCounterValue = 0;
	private int channelCountMax;
	private JLabel lblNewLabel_2;
	
	/**
	 * 
	 * Build Date:2011-9-29 
	 * Author:Administrator
	 * Function Description: Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					BufferedPulseWidthMeasurement frame = new BufferedPulseWidthMeasurement();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * 
	 * Build Date:2011-9-29 
	 * Author:Administrator
	 * Function Description: Create the frame.
	 */
	public BufferedPulseWidthMeasurement() {
		setResizable(false);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 492, 551);
		addWindowListener(new WindowCloseActionListener());
		
		contentPane = new JPanel();
		contentPane.setBackground(SystemColor.control);
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);
		
		txtChannelStatus = new JTextField();
		txtChannelStatus.setHorizontalAlignment(SwingConstants.RIGHT);
		txtChannelStatus.setEditable(false);
		txtChannelStatus.setBounds(111, 116, 149, 21);
		contentPane.add(txtChannelStatus);
		txtChannelStatus.setColumns(10);
		
		lstCounterValue = new JList();
		lstCounterValue.setBackground(SystemColor.controlHighlight);
		lstCounterValue.setBorder(new LineBorder(new Color(0, 0, 0)));
		lstCounterValue.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		lstCounterValue.setBounds(25, 160, 302, 287);
		lstCounterValue.setAlignmentX(Component.RIGHT_ALIGNMENT);
		lstCounterValue.setFixedCellHeight(28);
		contentPane.add(lstCounterValue);
		
		btnStart = new JButton("Start");
		btnStart.setBounds(351, 182, 90, 23);
		btnStart.addActionListener(new ButtonStartActionListener());
		contentPane.add(btnStart);
		
		btnStop = new JButton("Stop");
		btnStop.setBounds(351, 261, 90, 23);
		btnStop.addActionListener(new ButtonStopActionListener());
		btnStop.setEnabled(false);
		contentPane.add(btnStop);
		
		btnConfigure = new JButton("Configure");
		btnConfigure.setBounds(351, 333, 90, 23);
		btnConfigure.addActionListener(new ButtonConfigureActionListener());
		contentPane.add(btnConfigure);
		
		// Add background image for the main Frame
		BackgroundPanel panel = new BackgroundPanel("Background.png", "background image!");
		panel.setBounds(311, 0, 175, 64);
		panel.setBackground(SystemColor.control);
		panel.setLayout(null);
		contentPane.add(panel);
		
		JLabel lblNewLabel = new JLabel("Channel:");
		lblNewLabel.setFont(new Font("Arial", Font.PLAIN, 15));
		lblNewLabel.setBounds(25, 70, 76, 29);
		contentPane.add(lblNewLabel);
		
		cbxDisplayedChannel = new JComboBox();
		cbxDisplayedChannel.setBounds(111, 70, 149, 21);
		cbxDisplayedChannel.addItemListener(new cbxDisplayedChannelItemListener());
		contentPane.add(cbxDisplayedChannel);
		
		JLabel lblNewLabel_1 = new JLabel("Status:");
		lblNewLabel_1.setFont(new Font("Arial", Font.PLAIN, 15));
		lblNewLabel_1.setBounds(25, 116, 71, 21);
		contentPane.add(lblNewLabel_1);
		
		lblNewLabel_2 = new JLabel("<html><body>Note:The example will display the first ten values on Data Ready Event every time.</body><html>");
		lblNewLabel_2.setBounds(25, 457, 309, 54);
		contentPane.add(lblNewLabel_2);
		contentPane.setFocusTraversalPolicy(new FocusTraversalOnArray(new Component[]{
				btnStart, btnStop, btnConfigure}));
	
		bfdPwMeterCtrl.addDataReadyListener(new DataReadyEventListener());
		bfdPwMeterCtrl.addStoppedListener(new StoppedEventListener());
		
		configureDialog = new ConfigureDialog(this);
		configureDialog.setModal(true);
		configureDialog.setVisible(true);
	}
	
	/**
	 * 
	 * Build Date:2011-9-29 
	 * Author:Administrator
	 * Function Description: This function is used to initialize the main frame.
	 */
	public void Initialization() {	
		//String array channelValues contents of the current channels' data in each channel.
		String[] channelValues = new String[MAX_ROW_COUNT];
		//set the main frame's title.
		this.setTitle("Buffered Pulse Width Measurement - Run(" + configure.deviceName.substring(0, configure.deviceName.length() - 1) + ")");
		
		model.removeAllElements();
		for (int i = 0; i < channelValues.length; i++) {
			channelValues[i] = "";
			model.addElement(channelValues[i]);
		}
		lstCounterValue.setModel(model);
		lstCounterValue.setBorder(new LineBorder(new Color(0, 0, 0)));
		lstCounterValue.setCellRenderer(new ListViewCellRenderer());
		lstCounterValue.setVisible(true);		
		txtChannelStatus.setText("");
		
		isRunning = false;
		haveData = false;

		ConfigureDevice();		
	}
	
	/**
	 * 
	 *Build Date:2011-9-29
	 *Author:Administrator
	 *Function Description: This function is used to configure the event counter device.
	 */
	private void ConfigureDevice(){
		try{
			//configure device
			bfdPwMeterCtrl.setSelectedDevice(new DeviceInformation(configure.deviceName));
			channelCountMax = bfdPwMeterCtrl.getFeatures().getChannelCountMax();
			channelEnable = new boolean [channelCountMax];
			ch_status = new String[channelCountMax];
			
			// ConfigureMode: ProfileConfigure / ManualConfigure
			if(configure.configureMode == 1){			
				String profilePath = null;
				profilePath = configureDialog.GetProfilePath();
				ErrorCode errorCode = bfdPwMeterCtrl.LoadProfile(profilePath);
				CheckError(errorCode);
				channelEnable = bfdPwMeterCtrl.getChannelMap();
				sectionLength = bfdPwMeterCtrl.getRecord().getSectionLength();
				sectionCount = bfdPwMeterCtrl.getRecord().getSectionCount();			
			}else{
				System.arraycopy(configure.channelEnabled, 0, channelEnable, 0, configure.channelEnabled.length);
				sectionLength = configure.sectionLength;
				sectionCount = configure.sectionCount;
				
				// Device General Settings
				bfdPwMeterCtrl.setChannelMap(configure.channelEnabled);
				bfdPwMeterCtrl.setSampleClkSrc(configure.sampleClkSrc);
				bfdPwMeterCtrl.getRecord().setSectionLength(configure.sectionLength);
				bfdPwMeterCtrl.getRecord().setSectionCount(configure.sectionCount);
				
				// Trigger Settings
				int trgCount = bfdPwMeterCtrl.getFeatures().getTriggerCount(); 
				if(trgCount != 0){
					Trigger trg = bfdPwMeterCtrl.getTrigger();
					trg.setSource(configure.triggerSource);
					trg.setAction(configure.triggerAction);
					trg.setEdge(configure.triggerEdge);
					trg.setLevel(configure.triggerLevel);
				}
			}
			
			//configure dataBuf 
			samplesCount = sectionLength;
			
			if(sectionCount != 0){
				samplesCount *= sectionCount;
			}		
			if(samplesCount <= 1){
				samplesCount = 2;
			}
			dataBuf = new PulseWidth[samplesCount];
			chs_dataBuf = new PulseWidth[channelCountMax][samplesCount];
			
			for(int i = 0; i < samplesCount; i++){
				dataBuf[i] = new PulseWidth();
			}
			
			for(int row = 0; row < channelCountMax; row++){
				for(int column = 0; column < samplesCount; column++){
					chs_dataBuf[row][column] = new PulseWidth();
				}
			}
			
			cbxDisplayedChannel.removeAllItems();	
			cbxDisplayedChannel.setEnabled(false);
			//enum enabled channels for cbxEventChannel
			for(int i = 0; i < channelCountMax; i++){
				if(channelEnable[i]){
					cbxDisplayedChannel.addItem(String.valueOf(i));
				} 
			}
			cbxDisplayedChannel.setEnabled(true);
			cbxDisplayedChannel.setSelectedIndex(0);
			
		}catch(Exception ex){
			ShowMessage("Sorry, there're some errors occured: " + ex.toString());
		}
		for(int i = 0; i < channelCountMax; i++){
			if(channelEnable[i]){
				ChannelStatusStringChange(i, "Ready");
			}
		}
		displayChannel = Integer.parseInt(cbxDisplayedChannel.getSelectedItem().toString());
		DisplayChannelStatus(displayChannel);
	}
	
	/**
	 * 
	 * Build Date:2011-9-29 
	 * Author:Administrator
	 * Function Description: This function is overload the interface ActionListener,and 
	 * 						 is implementation need this function!
	 */
	@Override
	public void actionPerformed(ActionEvent e) {

	}
	
	/**
	 * 
	 * Build Date:2011-9-29 
	 * Author:Administrator
	 * Function Description: If some errors occurred, Show the error code to the users.
	 * 
	 * @param message:the message shown to users!
	 */
	protected void ShowMessage(String message) {
		JOptionPane.showMessageDialog(this, message, "Warning MessageBox",
				JOptionPane.WARNING_MESSAGE);
	}
	
	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: check errorcode
	 */
	protected void CheckError(ErrorCode errorCode)
	{
		if(Global.BioFaild(errorCode)){
			ShowMessage("Sorry, there're some errors occred, ErrorCode: " + errorCode.toString());
		}		
	}
	
	protected void ChannelStatusStringChange(int channel, String str)
	{
		ch_status[channel] = str;
	}
	
	protected void DisplayChannelStatus(int channel)
	{
		txtChannelStatus.setText("");
		txtChannelStatus.setText(ch_status[channel]);
	}
	
	private void DisplayChannelValue(int channel)
	{	
		listCount = MAX_ROW_COUNT;
		if(sectionLength < MAX_ROW_COUNT){
			listCount = sectionLength;
		}
		
		if(isRunning | haveData){
			for(int i = 0; i < listCount; i++){				
				model.set(i, "High: " + String.format("%.6f", chs_dataBuf[channel][i].HiPeriod) + " s   Low: " + String.format("%.6f", chs_dataBuf[channel][i].LoPeriod) + " s");
			}
		}
	}
	
	protected void CopyChannelData(int channel)
	{
		int ch = channel;
		System.arraycopy(dataBuf, 0, chs_dataBuf[ch], 0, dataBuf.length);
	}
	
	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: this class is use to draw each cell of the JList object
	 */
	class ListViewCellRenderer extends JLabel implements ListCellRenderer {
		/**
		 * define the serialization number.
		 */
		private static final long serialVersionUID = 1L;

		public Component getListCellRendererComponent(JList list, Object value, int index,
				boolean isSelected, boolean cellHasFocus) {

			if (value != null) {
				String text = value.toString();
				setText(text);
				Border etch = BorderFactory.createEtchedBorder();
				this.setBorder(BorderFactory.createTitledBorder(etch));

				setHorizontalAlignment(SwingConstants.RIGHT);
				setBackground(SystemColor.controlHighlight);
				setOpaque(true);
			}
			return this;
		}
	}
	
	/**
	 * 
	 * @author Administrator
	 * Class Function Description: This class is used to listen the start button 's action.
	 */
	class ButtonStartActionListener implements ActionListener{
		public void actionPerformed(ActionEvent e) {
			ErrorCode errorCode = ErrorCode.Success;
			isRunning = true;
			haveData = false;
			for(int i = 0; i < channelCountMax; i++){
				if(channelEnable[i]){
					ChannelStatusStringChange(i, "Running");
				}
			}
			for(int i = 0; i < model.getSize(); i++){
				model.set(i, "");
			}
			
			{		
				errorCode = bfdPwMeterCtrl.Prepare();
				CheckError(errorCode);
				
				try {
					bfdPwMeterCtrl.setEnabled(true);
				} catch (Exception ex) {
					ShowMessage("Sorry, there're some errors occured: " + ex.toString());
					return;
				}
				
				DisplayChannelStatus(displayChannel);
				
				btnConfigure.setEnabled(false);
				btnStart.setEnabled(false);
				btnStop.setEnabled(true);
			}
		}
	}
	
	/**
	 * 
	 * @author Administrator
	 * Class Function Description: This class is used to listen the stop button 's action.
	 */
	class ButtonStopActionListener implements ActionListener{
		public void actionPerformed(ActionEvent e) {
			try {
				bfdPwMeterCtrl.setEnabled(false);
			} catch (Exception ex) {
				ShowMessage("Sorry, there're some errors occured: " + ex.toString());
				return;
			}
			
			isRunning = false;
			btnStart.setEnabled(true);
			btnConfigure.setEnabled(true);
			btnStop.setEnabled(false);
			
			for(int i = 0; i < channelCountMax; i++){
				if(channelEnable[i]){
					ChannelStatusStringChange(i, "Stopped");
				}
			}
		}
	}
	
	/**
	 * 
	 * @author Administrator
	 * Class Function Description: This class is used to listen the configure button 's action.
	 */
	class ButtonConfigureActionListener implements ActionListener{
		public void actionPerformed(ActionEvent e) {
			if(bfdPwMeterCtrl.getEnabled()){
				try {
					bfdPwMeterCtrl.setEnabled(false);
				} catch (Exception ex) {
					ShowMessage("Sorry, there're some errors occured: " + ex.toString());
					return;
				}
			}
			haveData = false;
			configureDialog.LoadConfiguration(configure);
			configureDialog.isFirstLoad = false;
			configureDialog.setVisible(true);
		}
	}
	
	class cbxDisplayedChannelItemListener implements ItemListener{

		@Override
		public void itemStateChanged(ItemEvent arg0) {
			// TODO Auto-generated method stub
			if(arg0.getStateChange() == ItemEvent.SELECTED)
            {
				displayChannel = Integer.parseInt(cbxDisplayedChannel.getSelectedItem().toString());
            }
			
			if(cbxDisplayedChannel.isEnabled()){
			    DisplayChannelStatus(displayChannel);
			    DisplayChannelValue(displayChannel);
			}
		}
	}
	
	public class DataReadyEventListener implements BfdCntrEventListener {
		@Override
		public void BfdCntrEvent(Object arg0, BfdCntrEventArgs arg1) {
			// TODO Auto-generated method stub
			int getDataCount = Math.min(arg1.Count, samplesCount);
			eventChannel = arg1.Channel;
			ErrorCode ret = bfdPwMeterCtrl.GetData(arg1.Channel, getDataCount, dataBuf, 0, null, null);
			if(Global.BioFaild(ret)){
				ShowMessage("Sorry, there're some errors occred, ErrorCode: " + ret.toString());
				return;
			}
			CopyChannelData(eventChannel);
			if(eventChannel == displayChannel){
				DisplayChannelValue(displayChannel);
			}
		}
	}
	
	public class StoppedEventListener implements BfdCntrEventListener {

		@Override
		public void BfdCntrEvent(Object arg0, BfdCntrEventArgs arg1) {
			// TODO Auto-generated method stub
			int getDataCount = Math.min(arg1.Count, samplesCount);
			eventChannel = arg1.Channel;
			if(getDataCount > 0){
				ErrorCode ret = bfdPwMeterCtrl.GetData(arg1.Channel, getDataCount, dataBuf, 0, null, null);
				if(Global.BioFaild(ret)){
					ShowMessage("Sorry, there're some errors occred, ErrorCode: " + ret.toString());
					return;
				}
				CopyChannelData(eventChannel);
				if(eventChannel == displayChannel){
					DisplayChannelValue(displayChannel);
				}
			}		
			
			if(bfdPwMeterCtrl.getState() == ControlState.Stopped){
				try {
					bfdPwMeterCtrl.setEnabled(false);
				} catch (Exception ex) {
					ShowMessage("Sorry, there're some errors occured: " + ex.toString());
					return;
				}
				
				isRunning = false;
				btnStart.setEnabled(true);
				btnConfigure.setEnabled(true);
				btnStop.setEnabled(false);
			}
			
			ChannelStatusStringChange(eventChannel, "Stopped");		
			DisplayChannelStatus(displayChannel);
		}
		
	}
	
	/**
	 * 
	 * @author Administrator
	 * Class Function Description: This class is used to listen the main frame's closing event.
	 */
	class WindowCloseActionListener extends WindowAdapter{
		@Override
		public void windowClosing(WindowEvent e) {
			if (bfdPwMeterCtrl.getEnabled()) {
				try {
					bfdPwMeterCtrl.setEnabled(false);
					bfdPwMeterCtrl.Cleanup();
				} catch (Exception ex) {
					// do nothing here.
					// ShowMessage("Sorry, there're some errors occurred: " + ex.toString());
				}
			}
		}
	}
}

/**
 * 
 * @author Administrator 
 * Class Description: this class is use to transfer parameter to the main frame !
 */
class ConfigureParameter {
	public String deviceName;
	public int sectionLength;
	public int sectionCount;
	public SignalDrop sampleClkSrc;
	
	public SignalDrop triggerSource;
	public TriggerAction triggerAction;	
	public ActiveSignal triggerEdge;
	public double triggerLevel;
	
	public boolean[] channelEnabled;
	
	public int configureMode; //load profile --> 1  /  manual configure --> 0
}
