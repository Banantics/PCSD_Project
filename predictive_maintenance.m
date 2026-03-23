% =========================================================================
% PSCD - Predictive Maintenance System
% MATLAB Serial Data Acquisition, FFT & Anomaly Detection
% Group 1 - Bogdan Ghetu
%
% HOW TO USE:
%   1. Connect STM32 Nucleo-144 via USB (ST-Link)
%   2. Make sure the correct COM port is set below (default: COM3)
%   3. Press Run (or type 'predictive_maintenance' in the Command Window)
%   4. Follow the on-screen prompts:
%        - Press ENTER to start baseline calibration
%        - Press ENTER again to start monitoring
%        - Close the figure window to stop
% =========================================================================

clc; clear; close all;

% -------------------------------------------------------------------------
% CONFIGURATION  <-- edit these if needed
% -------------------------------------------------------------------------
COM_PORT     = 'COM5';      % Serial port of the STM32 ST-Link
BAUD_RATE    = 115200;      % Must match STM32 firmware
SAMPLE_RATE  = 1000;        % Hz - accelerometer sample rate
BLOCK_SIZE   = 1024;        % Samples per FFT block (must match firmware)
THRESHOLD    = 50;          % Euclidean distance alarm threshold (tune in Q4)
BYTES_PER_SAMPLE = 2;       % STM32 sends int16 (2 bytes per sample)

% -------------------------------------------------------------------------
% SERIAL PORT SETUP
% -------------------------------------------------------------------------
fprintf('=== PSCD Predictive Maintenance System ===\n');
fprintf('Connecting to STM32 on %s at %d baud...\n', COM_PORT, BAUD_RATE);

s = serialport(COM_PORT, BAUD_RATE);
configureTerminator(s, "LF");   % Line-feed terminated (adjust if firmware differs)
flush(s);
fprintf('Connected.\n\n');

% Clean up serial port automatically when script exits or errors
cleanupObj = onCleanup(@() delete(s));

% -------------------------------------------------------------------------
% FIGURE SETUP
% -------------------------------------------------------------------------
fig = figure('Name', 'PSCD Predictive Maintenance', ...
             'NumberTitle', 'off', ...
             'Position', [100 100 1100 700]);

% --- Time domain plot ---
ax1 = subplot(2,2,1);
hTime = plot(ax1, zeros(1, BLOCK_SIZE), 'b-', 'LineWidth', 1);
title(ax1, 'Raw Vibration Signal');
xlabel(ax1, 'Sample');
ylabel(ax1, 'Amplitude (raw)');
grid(ax1, 'on');
ylim(ax1, [-2^15, 2^15]);   % int16 range

% --- Live FFT plot ---
freqAxis = (0:BLOCK_SIZE/2) * (SAMPLE_RATE / BLOCK_SIZE);
ax2 = subplot(2,2,2);
hFFT    = plot(ax2, freqAxis, zeros(1, BLOCK_SIZE/2+1), 'b-', 'LineWidth', 1);
hold(ax2, 'on');
hBase   = plot(ax2, freqAxis, zeros(1, BLOCK_SIZE/2+1), 'r--', 'LineWidth', 1.5);
hold(ax2, 'off');
legend(ax2, 'Live FFT', 'Baseline');
title(ax2, 'FFT Spectrum');
xlabel(ax2, 'Frequency (Hz)');
ylabel(ax2, 'Magnitude');
grid(ax2, 'on');

% --- Euclidean distance history ---
ax3 = subplot(2,2,3);
hDist = plot(ax3, NaN, NaN, 'k-', 'LineWidth', 1.2);
hold(ax3, 'on');
hThreshLine = yline(ax3, THRESHOLD, 'r--', 'LineWidth', 1.5, ...
                    'Label', sprintf('Threshold = %g', THRESHOLD));
hold(ax3, 'off');
title(ax3, 'Euclidean Distance to Baseline');
xlabel(ax3, 'Block #');
ylabel(ax3, 'Distance');
grid(ax3, 'on');
ylim(ax3, [0, THRESHOLD * 3]);

% --- Status panel ---
ax4 = subplot(2,2,4);
axis(ax4, 'off');
hStatus = text(ax4, 0.5, 0.5, 'STATE: IDLE', ...
    'FontSize', 22, 'FontWeight', 'bold', ...
    'HorizontalAlignment', 'center', ...
    'VerticalAlignment', 'middle', ...
    'Color', [0.4 0.4 0.4]);
hDistText = text(ax4, 0.5, 0.25, 'Distance: --', ...
    'FontSize', 14, ...
    'HorizontalAlignment', 'center', ...
    'Color', [0.2 0.2 0.2]);

% -------------------------------------------------------------------------
% STATE MACHINE INITIALISATION
% -------------------------------------------------------------------------
% States: 1=IDLE, 2=CALIBRATION, 3=MONITORING, 4=ALARM
state         = 1;
baseline      = [];
distHistory   = [];
blockCount    = 0;

updateStatus(hStatus, state);

% -------------------------------------------------------------------------
% MAIN LOOP
% -------------------------------------------------------------------------
fprintf('Press ENTER in this window to begin baseline calibration...\n');
pause();

state = 2;  % -> CALIBRATION
updateStatus(hStatus, state);
fprintf('\n[CALIBRATION] Recording baseline. Press ENTER when ready to switch to monitoring...\n');
calibrationBlocks = {};

while ishandle(fig)

    % --- Read one block of samples from serial port ---
    rawSamples = readBlock(s, BLOCK_SIZE, BYTES_PER_SAMPLE);
    if isempty(rawSamples)
        continue;
    end

    blockCount = blockCount + 1;

    % --- Compute FFT magnitude (single-sided) ---
    N       = length(rawSamples);
    Y       = fft(double(rawSamples));
    P2      = abs(Y / N);
    P1      = P2(1:N/2+1);
    P1(2:end-1) = 2 * P1(2:end-1);   % single-sided scaling

    % --- Update time domain plot ---
    set(hTime, 'YData', double(rawSamples));

    % --- Update FFT plot ---
    set(hFFT, 'YData', P1);
    if ~isempty(baseline)
        set(hBase, 'YData', baseline);
    end

    % ---- STATE LOGIC ----
    switch state

        case 2  % CALIBRATION
            calibrationBlocks{end+1} = P1; %#ok<AGROW>

            % Check if user pressed ENTER to finish calibration
            if ~isempty(get(0, 'CurrentFigure')) && ...
               strcmpi(get(fig, 'CurrentKey'), 'return')
                % Average all recorded blocks as the baseline
                baseline = mean(cat(1, calibrationBlocks{:}), 1);
                set(hBase, 'YData', baseline);
                state = 3;  % -> MONITORING
                updateStatus(hStatus, state);
                fprintf('\n[MONITORING] Baseline stored from %d blocks. Monitoring started.\n', ...
                        length(calibrationBlocks));
                set(fig, 'CurrentKey', '');  % reset key
            end

        case {3, 4}  % MONITORING or ALARM

            if isempty(baseline)
                continue;
            end

            % --- Euclidean distance ---
            d = sqrt(sum((P1 - baseline).^2));
            distHistory(end+1) = d; %#ok<AGROW>

            % Update distance plot
            set(hDist, 'XData', 1:length(distHistory), 'YData', distHistory);

            % Update distance text
            set(hDistText, 'String', sprintf('Distance: %.2f', d));

            % State transitions
            if d > THRESHOLD
                state = 4;  % -> ALARM
            else
                state = 3;  % -> MONITORING (or back from ALARM)
            end
            updateStatus(hStatus, state);

            if state == 4
                fprintf('[ALARM] Block %d: Euclidean distance = %.2f (threshold = %g)\n', ...
                        blockCount, d, THRESHOLD);
                % Send alarm command back to STM32 (LED on)
                writeline(s, 'ALARM_ON');
            else
                % Send clear command to STM32 (LED off)
                writeline(s, 'ALARM_OFF');
            end
    end

    drawnow limitrate;

    % Check for ENTER key press to transition IDLE -> CALIBRATION
    % (handled above) or quit
    if ~ishandle(fig)
        break;
    end
end

fprintf('\nSession ended. Serial port closed.\n');

% =========================================================================
% HELPER FUNCTIONS
% =========================================================================

function samples = readBlock(s, blockSize, bytesPerSample)
% Read one complete block of int16 samples from the serial port.
% Returns [] if not enough bytes are available yet.
    totalBytes = blockSize * bytesPerSample;
    if s.NumBytesAvailable < totalBytes
        samples = [];
        return;
    end
    raw     = read(s, totalBytes, 'uint8');
    % Reassemble int16 from two bytes (little-endian, matching STM32 output)
    lo      = double(raw(1:2:end));
    hi      = double(raw(2:2:end));
    samples = int16(lo + hi * 256);
end

function updateStatus(hText, state)
% Update the status text box colour and label based on current state.
    switch state
        case 1
            set(hText, 'String', 'STATE: IDLE',        'Color', [0.4 0.4 0.4]);
        case 2
            set(hText, 'String', 'STATE: CALIBRATION', 'Color', [0.1 0.5 0.9]);
        case 3
            set(hText, 'String', 'STATE: MONITORING',  'Color', [0.1 0.7 0.2]);
        case 4
            set(hText, 'String', 'STATE: ALARM',        'Color', [0.9 0.1 0.1]);
    end
end
