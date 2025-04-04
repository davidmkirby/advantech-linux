package AI_SynchronousOneBufferedAI;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.math.BigDecimal;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.ListCellRenderer;
import javax.swing.SwingConstants;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.border.LineBorder;
import javax.swing.JTextField;
import javax.swing.JSlider;

import java.awt.SystemColor;

import javax.swing.JButton;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import Automation.BDaq.*;
import Common.*;

import org.eclipse.wb.swing.FocusTraversalOnArray;

public class SynchronousOneBufferedAI extends JFrame {
	// define the serialization number.
	private static final long serialVersionUID = 1L;

	private JPanel contentPane;
	private SimpleGraph graph;
	private JLabel label_YCoordinateMax;
	private JLabel label_YCoordinateMid;
	private JLabel label_YCoordinateMin;
	private JLabel label_XCoordinateMin;
	private JLabel label_XCoordinateMax;
	private JList listView;
	private JTextField txtShift;
	private JLabel lblShift;
	private JSlider sliderShift;
	private JTextField txtDiv;
	private JLabel lblDiv;
	private JSlider sliderDiv;
	private JButton btnConfigure;
	private JButton btnGetData;
	private DefaultListModel model = new DefaultListModel();

	private WaveformAiCtrl wfAiCtrl = new WaveformAiCtrl();
	
	ConfigureParameter configure = new ConfigureParameter();
	private TimeUnit timeUnit = TimeUnit.Millisecond;
	private double[] dataScaled;
	private ConfigureDialog configureDialog;

	/**
	 * Build Date:2011-8-15 
	 * Author:Administrator 
	 * Function Description: Launch the application.
	 */
	public static void main(String[] args) {

		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					SynchronousOneBufferedAI frame = new SynchronousOneBufferedAI();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Build Date:2011-8-15 
	 * Author:Administrator 
	 * Function Description: Create the main frame
	 */
	public SynchronousOneBufferedAI() {
		// Add window close action listener.
		addWindowListener(new WindowCloseActionListener());
		
		setTitle("Synchronous One Buffered AI - Run");
		setResizable(false);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 766, 522);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);

		// Add background image for the main Frame
		BackgroundPanel panel = new BackgroundPanel("Background.png", "background image!");
		panel.setBounds(0, 0, 760, 492);
		contentPane.add(panel);
		panel.setLayout(null);

		graph = new SimpleGraph();
		graph.setBackground(Color.BLACK);
		graph.setBounds(49, 39, 660, 324);
		graph.size = new Dimension(graph.getSize());
		panel.add(graph);

		label_YCoordinateMax = new JLabel("5V");
		label_YCoordinateMax.setHorizontalAlignment(SwingConstants.RIGHT);
		label_YCoordinateMax.setBounds(0, 39, 46, 15);
		panel.add(label_YCoordinateMax);

		label_YCoordinateMid = new JLabel("0");
		label_YCoordinateMid.setHorizontalAlignment(SwingConstants.RIGHT);
		label_YCoordinateMid.setBounds(0, 193, 46, 15);
		panel.add(label_YCoordinateMid);

		label_YCoordinateMin = new JLabel("-5V");
		label_YCoordinateMin.setHorizontalAlignment(SwingConstants.RIGHT);
		label_YCoordinateMin.setBounds(0, 348, 46, 15);
		panel.add(label_YCoordinateMin);

		label_XCoordinateMin = new JLabel("0Sec");
		label_XCoordinateMin.setHorizontalAlignment(SwingConstants.LEFT);
		label_XCoordinateMin.setBounds(49, 367, 70, 15);
		panel.add(label_XCoordinateMin);

		label_XCoordinateMax = new JLabel("10Sec");
		label_XCoordinateMax.setHorizontalAlignment(SwingConstants.RIGHT);
		label_XCoordinateMax.setBounds(629, 364, 80, 15);
		panel.add(label_XCoordinateMax);

		JLabel lblColorOfChannels = new JLabel("<html><body>Color of<br>channels:</body></html>");
		lblColorOfChannels.setHorizontalAlignment(SwingConstants.LEFT);
		lblColorOfChannels.setBounds(49, 383, 64, 38);
		panel.add(lblColorOfChannels);

		listView = new JList();
		listView.setBorder(new LineBorder(new Color(0, 0, 0)));
		listView.setBounds(111, 386, 475, 38);
		listView.setLayoutOrientation(JList.HORIZONTAL_WRAP);
		listView.setDoubleBuffered(true);
		listView.setAlignmentX(Component.RIGHT_ALIGNMENT);
		listView.setFixedCellWidth(59);
		listView.setFixedCellHeight(19);
		panel.add(listView);

		JLabel label_1 = new JLabel("Shift:");
		label_1.setBounds(49, 439, 46, 15);
		panel.add(label_1);

		txtShift = new JTextField();
		txtShift.setEditable(false);
		txtShift.setText("0");
		txtShift.setHorizontalAlignment(SwingConstants.RIGHT);
		txtShift.setBounds(94, 436, 70, 21);
		panel.add(txtShift);
		txtShift.setColumns(10);

		sliderShift = new JSlider();
		sliderShift.setEnabled(false);
		sliderShift.setBackground(SystemColor.control);
		sliderShift.setBounds(191, 436, 128, 25);
		sliderShift.addChangeListener(new SliderShiftChangeListener());
		panel.add(sliderShift);

		lblShift = new JLabel("ms");
		lblShift.setBounds(168, 439, 22, 15);
		panel.add(lblShift);

		JLabel label_2 = new JLabel("Div:");
		label_2.setBounds(329, 439, 30, 15);
		panel.add(label_2);

		txtDiv = new JTextField();
		txtDiv.setEditable(false);
		txtDiv.setText("200");
		txtDiv.setHorizontalAlignment(SwingConstants.RIGHT);
		txtDiv.setColumns(10);
		txtDiv.setBounds(359, 436, 70, 21);
		panel.add(txtDiv);

		lblDiv = new JLabel("ms");
		lblDiv.setBounds(434, 439, 22, 15);
		panel.add(lblDiv);

		sliderDiv = new JSlider();
		sliderDiv.setEnabled(false);
		sliderDiv.addChangeListener(new SliderDivChangeListener());
		sliderDiv.setBackground(SystemColor.control);
		sliderDiv.setBounds(459, 436, 128, 25);
		panel.add(sliderDiv);

		btnConfigure = new JButton("Configure");
		btnConfigure.addActionListener(new ButtonConfigureActionListener());
		btnConfigure.setBounds(596, 398, 115, 23);
		panel.add(btnConfigure);

		btnGetData = new JButton("Get Data");
		btnGetData.addActionListener(new ButtonGetDataActionListener());
		btnGetData.setBounds(597, 435, 112, 23);
		panel.add(btnGetData);
		panel.setFocusTraversalPolicy(new FocusTraversalOnArray(new Component[]{
				sliderShift, sliderDiv, btnConfigure, btnGetData}));

		configureDialog = new ConfigureDialog(this);
		configureDialog.setModal(true);
		configureDialog.setVisible(true);
	}

	/**
	 * 
	 * Build Date:2011-8-16 
	 * Author:Administrator 
	 * Function Description: Initialize the main frame.
	 */
	public void Initialization() {
		ConfigureDevice();
		ConfigureGraph();
		
		/*
		 * String array channelValues contents of the current channels' data in
		 * each channel.
		 */
		String[] channelValues = new String[configure.channelCount];

		this.setTitle("Synchronous One Buffered AI - Run(" + configure.deviceName.substring(0, configure.deviceName.length() - 1) + ")");

		model.removeAllElements();
		for (int i = 0; i < channelValues.length; i++) {
			channelValues[i] = "";
			model.addElement(channelValues[i]);
		}
		listView.setModel(model);
		listView.setBorder(new LineBorder(new Color(0, 0, 0)));
		listView.setVisibleRowCount(-1);
		listView.setCellRenderer(new ListViewCellRenderer());
		listView.setVisible(true);
	}

	/**
	 * 
	 * Build Date:2011-8-16 
	 * Author:Administrator 
	 * Function Description: Configure the Device.
	 */
	protected void ConfigureDevice() {
		try {
			wfAiCtrl.setSelectedDevice(new DeviceInformation(configure.deviceName));
			String profilePath = null;
			profilePath = configureDialog.GetProfilePath();
			ErrorCode errorCode = wfAiCtrl.LoadProfile(profilePath);
			if(Global.BioFaild(errorCode)){
				ShowMessage("Sorry, there're some errors occred, ErrorCode: " + errorCode.toString());
				}
			
			//Set the streaming mode.
			AnalogInputChannel[] channels = wfAiCtrl.getChannels();
			wfAiCtrl.getConversion().setChannelStart(configure.channelStart);
			wfAiCtrl.getConversion().setChannelCount(configure.channelCount);
			wfAiCtrl.getRecord().setSectionLength(configure.sectionLength);
			wfAiCtrl.getRecord().setSectionCount(1);//The nonzero value means 'one buffered' mode;
			wfAiCtrl.getConversion().setClockRate(configure.clockRatePerChan);
			int count = wfAiCtrl.getFeatures().getChannelCountMax();
			
			int channel = configure.channelStart;
			
			for (int i = 0; i < configure.channelCount; ++i)
		    {
		      if (channel >= count){
		         channel = 0;}
		      if (channels[channel].getSignalType() == AiSignalType.Differential)
		      {
		         if (channel%2 == 1){
		            channel -= 1;}
		         channels[channel%count].setValueRange((ValueRange)(configure.valueRange));
		         channel += 1;
		      }
		      channels[channel%count].setValueRange((ValueRange)(configure.valueRange));
		      channel += 1;
		    }
		} catch (Exception ex) {
			ShowMessage("Sorry, there're some errors occred: " + ex.getMessage());
		}
		
		ErrorCode errorCode = wfAiCtrl.Prepare();
		if(Global.BioFaild(errorCode)){
			ShowMessage("Sorry, there're some errors occred, ErrorCode: " + errorCode.toString());
		}
		
		/**
		 * the channel start, channel count and clock rate are not always configured follow the value 
		 * users have selected. Such as 'PCI-1714', its channel start cann't be 1,3, channel
		 * count cann't be 3 and clock rate must bigger than 250000.
		 */
		int channelStart = wfAiCtrl.getConversion().getChannelStart();
		int channelCount = wfAiCtrl.getConversion().getChannelCount();
		double clockRate = wfAiCtrl.getConversion().getClockRate();
		
		if (channelStart != configure.channelStart) {
			ShowMessage("Sorry, the channel start is invalid, driver will change it from "
					+ configure.channelStart + " to " + channelStart);
			configure.channelStart = channelStart;
		}

		if(channelCount != configure.channelCount){
			ShowMessage("Sorry, the channel count is invalid, driver will change it from "
					+ configure.channelCount + " to " + channelCount);
			configure.channelCount = channelCount;
		}
		
		if(clockRate != configure.clockRatePerChan){
			configure.clockRatePerChan = clockRate;
		}
	}

	/**
	 * 
	 * Build Date:2011-8-15 
	 * Author:Administrator 
	 * Function Description: Configure the simple graph.
	 */
	protected void ConfigureGraph() {
		double clockRate = wfAiCtrl.getConversion().getClockRate();
		int unit = 2;
		double timeInterval = 100.0 * graph.size.width / clockRate;
		double shiftMax = 1000.0 * wfAiCtrl.getRecord().getSectionLength() / clockRate;
		
		while (clockRate >= 10 * 1000) {
			timeInterval *= 1000;
			clockRate /= 1000;
			unit = unit - 1;
			shiftMax *= 1000;
			} 		

		sliderDiv.setMaximum((int)(4 * timeInterval));
		sliderDiv.setMinimum((int)Math.ceil(timeInterval / 10));
		sliderDiv.setValue((int)(timeInterval));
		
		sliderShift.setMaximum((int)shiftMax);
		sliderShift.setMinimum(0);
		sliderShift.setValue(0);
		txtShift.setText(String.valueOf(sliderShift.getValue()));

		TimeUnit[] tUnits = {TimeUnit.Nanosecond, TimeUnit.Microsecond, TimeUnit.Millisecond, TimeUnit.Second};
		timeUnit = tUnits[unit];
		
		String[] timeUnit = {"ns", "us", "ms", "Sec"};
		lblShift.setText(timeUnit[unit]);
		lblDiv.setText(timeUnit[unit]);	
		
		SetXCord();

		StringBuffer description = new StringBuffer(BDaqApi.VALUE_RANGE_DESC_MAX_LEN);
		MathInterval range = new MathInterval();
		IntByRef unitIndex = new IntByRef();

		/**
		 * String array Y ranges contents of three elements, They are the
		 * Maximum value of Y coordinate, the Middle value of Y coordinate and
		 * the Minimum value of Y coordinate!
		 */
		String[] Yranges = new String[3];
		BDaqApi.AdxGetValueRangeInformation(configure.valueRange.toInt(),description, range, unitIndex);
		Global.GetYCordRange(Yranges, range, Global.toValueUnit(unitIndex.value));
		
		this.label_YCoordinateMax.setText(Yranges[2]);
		this.label_YCoordinateMid.setText(Yranges[1]);
		this.label_YCoordinateMin.setText(Yranges[0]);
		if (ValueUnit.values()[unitIndex.value] == ValueUnit.Millivolt) {
			range.Max /= 1000;
			range.Min /= 1000;
		}
		
		graph.setyCordRangeMax(range.Max);
		graph.setyCordRangeMin(range.Min);
		graph.Clear();
	}

	/**
	 * 
	 * Build Date:2011-8-17 
	 * Author:Administrator 
	 * Function Description: This function is used to set the X coordinate of 
	 * 						 the simple graph.
	 */
	private void SetXCord() {
		graph.setXCordTimeDiv((double) (sliderDiv.getValue()));
		graph.setxCordTimeOffest((double) (sliderShift.getValue()));
		String[] X_rangeLabels = new String[2];
		double shiftMaxValue = round(graph.getXCordTimeDiv() * 10 + sliderShift.getValue(), 3,
				BigDecimal.ROUND_HALF_UP);
		Global.GetXCordRange(X_rangeLabels, shiftMaxValue, sliderShift.getValue(), timeUnit);
		label_XCoordinateMin.setText(X_rangeLabels[0]);
		label_XCoordinateMax.setText(X_rangeLabels[1]);
	}

	/**
	 * Build Date:2011-8-17 
	 * Author:Administrator 
	 * Function Description: This function is used to round a double value 
	 * 						 in 'scale' precision.
	 * 
	 * @param value: The double data
	 * @param scale: Precision number
	 * @param roundingMode: Rounding mode
	 * @return: the return value
	 */
	private double round(double value, int scale, int roundingMode) {
		BigDecimal bd = new BigDecimal(value);
		bd = bd.setScale(scale, roundingMode);
		double d = bd.doubleValue();
		bd = null;
		return d;
	}

	/**
	 * Build Date:2011-8-15 
	 * Author:Administrator 
	 * Function Description: if some errors occurred, Show the error code
	 *						 to the users.
	 * 
	 * @param message: the message shown to users.
	 */
	protected void ShowMessage(String message) {
		JOptionPane.showMessageDialog(this, message, "Warning MessageBox",
				JOptionPane.WARNING_MESSAGE);
	}

	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: This class is used to listen the shift slider's changing.
	 */
	class SliderShiftChangeListener implements ChangeListener {
		public void stateChanged(ChangeEvent arg0) {
			txtShift.setText(String.valueOf(sliderShift.getValue()));
			graph.Shift(sliderShift.getValue());
			
			SetXCord();
		}
	}

	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: This class is used to listen the Div slider's changing.
	 */
	class SliderDivChangeListener implements ChangeListener {
		public void stateChanged(ChangeEvent e) {
			txtDiv.setText(String.valueOf(sliderDiv.getValue()));
			graph.Div(sliderDiv.getValue());
			
			SetXCord();
		}
	}

	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: This class is used to listen the configure
	 * 							   button's action.
	 */
	class ButtonConfigureActionListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			graph.Clear();
			configureDialog.LoadConfiguration(configure);
			configureDialog.isFirstLoad = false;
			configureDialog.setVisible(true);
		}
	}

	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: This class is used to listen the get data 
	 * 							   button's action.
	 */
	class ButtonGetDataActionListener implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			//'channelCount * dataCountPerChan' stands our raw data buffer's size.
			int rawDataBufferLength = configure.channelCount * configure.sectionLength;
			if (dataScaled == null || dataScaled.length < rawDataBufferLength) {
				dataScaled = new double[rawDataBufferLength];
			}
			
			graph.Clear();

			ErrorCode errorCode = wfAiCtrl.Start();
			if (Global.BioFaild(errorCode)) {
				ShowMessage("Sorry, there're some errors occred, ErrorCode: " + errorCode.toString());
				return;
			} else {
				errorCode = wfAiCtrl.GetData(configure.channelCount * configure.sectionLength, dataScaled, -1, null, null, null, null);
				if (Global.BioFaild(errorCode)) {
					ShowMessage("Sorry, there're some errors occred, ErrorCode: " + errorCode.toString());
					return;
				}

				graph.Chart(dataScaled, configure.channelCount, configure.sectionLength,
						1.0 / configure.clockRatePerChan);
			}
			sliderShift.setEnabled(true);
			sliderDiv.setEnabled(true);
		}
	}

	/**
	 * 
	 * @author Administrator 
	 * Class Function Description: this class is use to draw each cell of
	 * 							   the JList object.
	 */
	class ListViewCellRenderer extends JLabel implements ListCellRenderer {
		/**
		 * define the serialization number.
		 */
		private static final long serialVersionUID = 1L;

		public Component getListCellRendererComponent(JList list, Object value,
				int index, boolean isSelected, boolean cellHasFocus) {

			if (value != null) {
				String text = value.toString();
				setText(text);
				Border etch = BorderFactory.createEtchedBorder();
				this.setBorder(BorderFactory.createTitledBorder(etch));

				setHorizontalAlignment(SwingConstants.RIGHT);
				setOpaque(true);
				setBackground(SimpleGraph.color[index]);
			}
			return this;
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
			if (wfAiCtrl != null) {
				wfAiCtrl.Cleanup();
			}
		}
	}
}

/**
 * 
 * @author Administrator 
 * Class Description: this class is use to transfer parameter to the main frame dialog.
 */
class ConfigureParameter {
	public String deviceName;
	public int channelStart;
	public int channelCount;
	public ValueRange valueRange;
	public double clockRatePerChan;
	public int sectionLength;
}
