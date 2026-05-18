% =========================================================================
% PSCD - Predictive Maintenance System
% MATLAB Serial Data Acquisition, FFT & Anomaly Detection
% Group 1 - Bogdan Ghetu
%
% HOW TO USE:
%   1. Plug in STM32 via USB
%   2. Press Run in MATLAB
%   3. Click the Command Window and press ENTER to start
%   4. System records baseline for 10 seconds automatically, then monitors
% =========================================================================

clc; clear; close all;

% -------------------------------------------------------------------------
% CONFIGURATION
% -------------------------------------------------------------------------
COM_PORT         = 'COM5';    % Change to match your port
BAUD_RATE        = 115200;
SAMPLE_RATE      = 500;       % Hz (approx, based on ~2ms between samples)
BLOCK_SIZE       = 512;       % Samples per FFT block
THRESHOLD        = 50;        % Euclidean distance alarm threshold (tune later)
AXIS             = 'Z';       % 'X', 'Y', or 'Z'
CALIB_SECONDS    = 10;        % How long to record baseline (seconds)

% -------------------------------------------------------------------------
% SERIAL PORT SETUP
% -------------------------------------------------------------------------
fprintf('=== PSCD Predictive Maintenance System ===\n');
fprintf('Connecting to STM32 on %s at %d baud...\n', COM_PORT, BAUD_RATE);
s = serialport(COM_PORT, BAUD_RATE);
configureTerminator(s, "LF");
flush(s);
fprintf('Connected successfully.\n\n');
cleanupObj = onCleanup(@() delete(s));

% -------------------------------------------------------------------------
% FIGURE
% -------------------------------------------------------------------------
fig = figure('Name','PSCD Predictive Maintenance','NumberTitle','off',...
             'Position',[100 100 1100 700]);
freqAxis = (0:BLOCK_SIZE/2) * (SAMPLE_RATE / BLOCK_SIZE);

ax1 = subplot(2,2,1);
hTime = plot(ax1, zeros(1,BLOCK_SIZE), 'b-', 'LineWidth', 1);
title(ax1, sprintf('Raw Vibration - %s axis', AXIS));
xlabel(ax1,'Sample'); ylabel(ax1,'Acceleration (raw)'); grid(ax1,'on');

ax2 = subplot(2,2,2);
hFFT  = plot(ax2, freqAxis, zeros(1,BLOCK_SIZE/2+1), 'b-', 'LineWidth', 1);
hold(ax2,'on');
hBase = plot(ax2, freqAxis, zeros(1,BLOCK_SIZE/2+1), 'r--', 'LineWidth', 1.5);
hold(ax2,'off');
legend(ax2,'Live FFT','Baseline');
title(ax2,'FFT Spectrum');
xlabel(ax2,'Frequency (Hz)'); ylabel(ax2,'Magnitude'); grid(ax2,'on');
set(ax2, 'YLimMode', 'auto', 'XLimMode', 'auto');

ax3 = subplot(2,2,3);
hDist = plot(ax3, NaN, NaN, 'k-', 'LineWidth', 1.2);
hold(ax3,'on');
yline(ax3, THRESHOLD, 'r--', 'LineWidth', 1.5, ...
      'Label', sprintf('Threshold = %g', THRESHOLD));
hold(ax3,'off');
title(ax3,'Euclidean Distance to Baseline');
xlabel(ax3,'Block #'); ylabel(ax3,'Distance'); grid(ax3,'on');
set(ax3, 'YLimMode', 'auto', 'XLimMode', 'auto');

ax4 = subplot(2,2,4);
axis(ax4,'off');
hStatus    = text(ax4, 0.5, 0.6, 'STATE: IDLE', 'FontSize', 22, ...
    'FontWeight','bold','HorizontalAlignment','center','Color',[0.4 0.4 0.4]);
hDistText  = text(ax4, 0.5, 0.35, 'Distance: --', 'FontSize', 14, ...
    'HorizontalAlignment','center','Color',[0.2 0.2 0.2]);
hBlockText = text(ax4, 0.5, 0.15, 'Block: 0', 'FontSize', 12, ...
    'HorizontalAlignment','center','Color',[0.4 0.4 0.4]);

% -------------------------------------------------------------------------
% WAIT FOR USER TO PRESS ENTER
% -------------------------------------------------------------------------
fprintf('Press ENTER in this window to begin %d-second baseline calibration...\n', CALIB_SECONDS);
pause();

% -------------------------------------------------------------------------
% CALIBRATION (timed - runs for CALIB_SECONDS automatically)
% -------------------------------------------------------------------------
updateStatus(hStatus, 2);
fprintf('\n[CALIBRATION] Recording baseline for %d seconds...\n', CALIB_SECONDS);

sampleBuffer = [];
calibBlocks  = {};
blockCount   = 0;
calibStart   = tic;

while toc(calibStart) < CALIB_SECONDS && ishandle(fig)

    while s.NumBytesAvailable > 0
        line = readline(s);
        vals = str2double(strsplit(strtrim(line), ','));
        if numel(vals) == 4 && ~any(isnan(vals))
            switch AXIS
                case 'X'; sampleBuffer(end+1) = vals(2); %#ok<AGROW>
                case 'Y'; sampleBuffer(end+1) = vals(3); %#ok<AGROW>
                case 'Z'; sampleBuffer(end+1) = vals(4); %#ok<AGROW>
            end
        end
    end

    while length(sampleBuffer) >= BLOCK_SIZE
        block        = sampleBuffer(1:BLOCK_SIZE);
        sampleBuffer = sampleBuffer(BLOCK_SIZE+1:end);
        blockCount   = blockCount + 1;

        Y  = fft(double(block));
        P2 = abs(Y / BLOCK_SIZE);
        P1 = P2(1:BLOCK_SIZE/2+1);
        P1(2:end-1) = 2 * P1(2:end-1);

        calibBlocks{end+1} = P1; %#ok<AGROW>
        set(hTime,      'YData', double(block));
        set(hFFT,       'YData', P1);
        set(hBlockText, 'String', sprintf('Calib blocks: %d', length(calibBlocks)));

        timeLeft = CALIB_SECONDS - toc(calibStart);
        fprintf('  Block %d recorded (%.1f seconds left)\n', length(calibBlocks), timeLeft);
    end

    drawnow limitrate;
    pause(0.02);
end

if isempty(calibBlocks)
    error('No data received during calibration. Check STM32 connection and COM port.');
end

% Lock in baseline
baseline = mean(cat(1, calibBlocks{:}), 1);
set(hBase, 'YData', baseline);
updateStatus(hStatus, 3);
fprintf('\n[MONITORING] Baseline locked from %d blocks. Anomaly detection running.\n\n', length(calibBlocks));

% -------------------------------------------------------------------------
% MONITORING LOOP
% -------------------------------------------------------------------------
state       = 3;
distHistory = [];

while ishandle(fig)

    while s.NumBytesAvailable > 0
        line = readline(s);
        vals = str2double(strsplit(strtrim(line), ','));
        if numel(vals) == 4 && ~any(isnan(vals))
            switch AXIS
                case 'X'; sampleBuffer(end+1) = vals(2); %#ok<AGROW>
                case 'Y'; sampleBuffer(end+1) = vals(3); %#ok<AGROW>
                case 'Z'; sampleBuffer(end+1) = vals(4); %#ok<AGROW>
            end
        end
    end

    while length(sampleBuffer) >= BLOCK_SIZE
        block        = sampleBuffer(1:BLOCK_SIZE);
        sampleBuffer = sampleBuffer(BLOCK_SIZE+1:end);
        blockCount   = blockCount + 1;

        Y  = fft(double(block));
        P2 = abs(Y / BLOCK_SIZE);
        P1 = P2(1:BLOCK_SIZE/2+1);
        P1(2:end-1) = 2 * P1(2:end-1);

        d = sqrt(sum((P1 - baseline).^2));
        distHistory(end+1) = d; %#ok<AGROW>

        set(hTime,     'YData', double(block));
        set(hFFT,      'YData', P1);
        set(hDist,     'XData', 1:length(distHistory), 'YData', distHistory);
        set(hDistText, 'String', sprintf('Distance: %.2f', d));
        set(hBlockText,'String', sprintf('Block: %d', blockCount));

        if d > THRESHOLD
            if state ~= 4
                fprintf('[ALARM] Block %d: distance = %.2f (threshold = %g)\n', blockCount, d, THRESHOLD);
                writeline(s, 'ALARM_ON');
            end
            state = 4;
        else
            if state == 4
                fprintf('[OK]    Block %d: distance = %.2f - back to normal\n', blockCount, d);
                writeline(s, 'ALARM_OFF');
            end
            state = 3;
        end
        updateStatus(hStatus, state);
    end

    drawnow limitrate;
    pause(0.02);
end

fprintf('Session ended.\n');

% =========================================================================
% HELPERS
% =========================================================================
function updateStatus(hText, state)
    switch state
        case 1; set(hText,'String','STATE: IDLE',        'Color',[0.4 0.4 0.4]);
        case 2; set(hText,'String','STATE: CALIBRATION', 'Color',[0.1 0.5 0.9]);
        case 3; set(hText,'String','STATE: MONITORING',  'Color',[0.1 0.7 0.2]);
        case 4; set(hText,'String','STATE: ALARM',        'Color',[0.9 0.1 0.1]);
    end
end
