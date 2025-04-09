# Combined Analog I/O Application

## Overview

This application provides a unified interface for controlling and monitoring Advantech data acquisition cards, specifically designed for the PCIE-1816 (AI/AO) and PCIE-1824 (AO) devices. It combines analog input (AI) streaming and analog output (AO) signal generation functionality into a single application.

## Features

- **Analog Input (AI)**
  - Real-time waveform display of multiple AI channels
  - Configurable sample rate and buffer sizes
  - Adjustable display scale and time divisions
  - Support for various value ranges
  - Event-driven data acquisition

- **Analog Output (AO)**
  - Generation of sine, triangle, and square waveforms
  - Configurable signal parameters (amplitude, frequency)
  - Manual voltage output mode
  - Support for multiple output channels

- **Integrated Features**
  - AI to AO pass-through for latency testing
  - Independent or synchronized AI/AO operation
  - Simple user interface with tabbed panels
  - Extensible architecture for future enhancements

## System Requirements

- Qt6 framework
- Advantech DAQNavi driver and SDK
- Advantech PCIE-1816 and/or PCIE-1824 cards
- Linux or Windows operating system

## Hardware Setup

### Supported Hardware

- **PCIE-1816**: Multifunction card supporting both AI and AO
- **PCIE-1824**: Dedicated AO card with high precision output

### Connections

For AI operation:
- Connect signal sources to analog input channels on the PCIE-1816

For AO operation:
- Connect oscilloscope or target device to analog output channels on either PCIE-1816 or PCIE-1824

For latency testing:
- Connect a function generator to an AI channel
- Connect an oscilloscope to both the input signal and the corresponding AO channel
- Enable the pass-through checkbox in the UI

## Software Architecture

The application is structured around these main components:

- **Configuration Dialog**: Manages device selection and parameter configuration
- **AI Module**: Handles streaming data acquisition and visualization
- **AO Module**: Provides waveform generation and output control
- **Device Management**: Handles initializing and controlling hardware devices

## Usage Guide

### First-Time Setup

1. Install the Advantech DAQNavi driver for your cards
2. Build the application using Qt6
3. Launch the application

### Device Configuration

1. When the application starts, a configuration dialog appears
2. Select your AI device (PCIE-1816) if available
3. Select your AO device (PCIE-1816 or PCIE-1824)
4. Configure parameters for each device
5. Click OK to proceed to the main application

### Using Analog Input

1. Select the "Analog Input" tab
2. Click "Start" to begin data acquisition
3. Adjust the time division using the slider
4. View real-time waveforms in the display
5. Click "Pause" to temporarily halt acquisition or "Stop" to end it

### Using Analog Output

1. Select the "Analog Output" tab
2. Choose a waveform type (sine, triangle, square) for each channel
3. Set high and low levels for the waveform
4. Alternatively, use manual mode to set a specific voltage
5. Adjust the timer frequency using the slider to control waveform speed

### Latency Testing

1. Ensure both AI and AO are configured
2. Connect a signal generator to an AI channel and an oscilloscope to monitor both input and output
3. Check the "Pass AI data to AO" checkbox in the AI tab
4. Start AI acquisition
5. Measure the delay between input and output signals on the oscilloscope

## Extending the Application

The application is designed to be extensible for future enhancements:

### Integration with Control Algorithms

The AI-to-AO pass-through feature provides a framework for inserting custom signal processing or control algorithms. The architecture supports:

- Real-time processing of acquired data
- Custom filters or controllers
- Feedback control systems
- Data analysis algorithms

### Adding New Signal Types

The AO waveform generation can be extended with:

- Additional waveform types
- Custom signal patterns
- Modulated signals
- Multi-channel synchronized outputs

## Troubleshooting

### Common Issues

1. **Device Not Found**
   - Ensure DAQNavi drivers are properly installed
   - Check PCI/PCIe slot connections
   - Verify device is recognized in DAQNavi Utility

2. **Signal Not Displayed**
   - Check connections to AI channels
   - Verify signal is within configured value range
   - Make sure acquisition is running (Start button pressed)

3. **No Output Generated**
   - Confirm AO device is properly configured
   - Check that a waveform type is selected (button is highlighted)
   - Verify output connections

4. **Application Crashes**
   - Check for out-of-range parameter values
   - Ensure hardware is properly installed and recognized
   - Look for error codes in the application output

## Performance Optimization

For optimal performance, especially when using the latency testing feature:

1. Set smaller section lengths for reduced latency
2. Use higher clock rates for faster sampling
3. Run the application on a system with minimal background processes
4. Consider dedicated hardware timing sync for precise measurements

## Future Development

Planned enhancements include:

- Integration with Simulink-generated controllers
- Addition of digital I/O support
- Expanded data logging capabilities
- Multi-device synchronization features
- Advanced signal processing options

---

*This application was developed for interfacing with Advantech data acquisition hardware. For more information about the supported devices, please refer to the Advantech documentation.*