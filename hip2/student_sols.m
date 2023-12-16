%NO_PFILE
function [funs, student_id] = student_sols()
%STUDENT_SOLS Contains all student solutions to problems.

% ----------------------------------------
%               STEP 1
% ----------------------------------------
% Set to your birthdate / the birthdate of one member in the group.
% Should a numeric value of format YYYYMMDD, e.g.
% student_id = 19900101;
% This value must be correct in order to generate a valid secret key.
student_id = 20021026;

% ----------------------------------------
%               STEP 2
% ----------------------------------------
% Your task is to implement the following skeleton functions.
% You are free to use any of the utility functions located in the same
% directory as this file as well as any of the standard matlab functions.

hip = load('hip2.mat');


    function h = gen_filter()
        
        %h = 0; %TODO: This line is missing some code!
        
        % Define the desired frequency response
        dt = 1;
        f_s = 1/dt;        % Sampling frequency (1 Hz)
        f_pass = 0.05;  % Passband frequency (0.05 Hz)
        f_stop = 0.1;   % Stopband frequency (0.1 Hz)
        
        % Calculate the filter order (number of coefficients)
        filter_order = 60;

        f = [0, f_pass, f_stop, f_s/2] /(f_s/2);      % *pi [rad/sample]
        a = [1, 1, 0, 0] .*f*pi *f_s;                 % filter amplitude
        h = firpm(filter_order,f,a,'differentiator');

        

        % Plot the resulting filter coefficients
        figure(6);
        stem(h);
        title('Filter Coefficients');
        xlabel('Coefficient Index');
        ylabel('Coefficient Value');
        % 
        % Plot the normalized magnitude of the frequency response
        figure(7);
        [H, W] = freqz(h, 1, 512);
        f_normalized = W / (2*pi*f_s/2);
        magnitude = abs(H);
        plot(f,a, f_normalized,magnitude,'LineWidth',1);
        legend('Ideal','firpm Design')
        title('Normalized Frequency Response');
        xlabel('Normalized Frequency');
        ylabel('Magnitude');
        grid on;
        % 
        %verify delay

        group_delay = grpdelay(h, 1, W)

        %question 4

        N = length(hip.true_position);

        fir_true_position = conv(hip.true_position, h) *3.6;
        comp_fir_true_position = fir_true_position(31:31+N-1); % delay 30s
        fir_noisy_position = conv(hip.noisy_position, h) *3.6;
        comp_fir_noisy_position = fir_noisy_position(31:31+N-1);
        %
        h_Euler = [1 -1];
        Euler_filter_true = conv(h_Euler,hip.true_position)*3.6;
        Euler_filter_noisy = conv(h_Euler,hip.noisy_position)*3.6;
        comp_euler_true = Euler_filter_true(2:end);
        comp_euler_noisy = Euler_filter_noisy(2:end);

        disp(['the maximum of the vehicle found from observed signal(noisy) is:',num2str(max(fir_noisy_position(31:N-1))),' km/h'])
        disp(['the maximum of the vehicle found from true signal(true) is:',num2str(max(fir_true_position(31:N-1))),' km/h'])

        figure(8);
        plot(fir_true_position)
        hold on;
        plot(fir_noisy_position)
        axis([0 600 -100 220])
        title('Estimation using designed filter');
        legend('true signal','noisy signal','FontSize', 15)
        xlabel('Time','FontSize', 15)
        ylabel('Estimated Speed (km/h)','FontSize', 15)

        lc = lines(6);
        % 
        figure(9);
        plot(comp_fir_true_position)
        hold on;
        plot(comp_fir_noisy_position)
        axis([0 600 -100 220])
        title('Estimation using designed filter after compensation for delay');
        legend('true signal','noisy signal','FontSize', 15)
        xlabel('Time','FontSize', 15)
        ylabel('Estimated Speed (km/h)','FontSize', 15)
        % 
        figure(10);
        xlabel 'time [s]', ylabel 'velocity [km/h]', hold on, grid on
        axis([0 600 0 220])
        plot(Euler_filter_true,'-','Color',lc(1,:),'LineWidth',1);
        plot(fir_noisy_position,'-','Color',lc(2,:),'LineWidth',1);
        plot(fir_true_position,'--','Color','k','LineWidth',1);
        title('Estimation using designed filter with euler filter');
        legend({'Euler - true', 'FIR - noisy','FIR - true'});
        % 
        figure(11);
        xlabel 'time [s]', ylabel 'velocity [km/h]', hold on, grid on
        axis([0 600 0 220])
        plot(comp_euler_true,'-','Color',lc(1,:),'LineWidth',1);
        plot(comp_fir_noisy_position,'-','Color',lc(2,:),'LineWidth',1);
        plot(comp_fir_true_position,'--','Color','k','LineWidth',1);
        title('Estimation using designed filter after compensation for delay with euler filter');
        legend({'Euler - true', 'FIR - noisy','FIR - true'});


        figure(12);
        plot(comp_euler_true)%compensate the polts for the delay 1s
        hold on;
        plot(comp_euler_noisy)%compensate the polts for the delay 1s
        axis([0 500 -1000 1000])
        title('Estimation using Euler filter');
        legend('true signal','noisy signal','FontSize', 15)
        xlabel('Time','FontSize', 15)
        ylabel('Estimated Speed (km/h)','FontSize', 15)


        y1 = [hip.noisy_position; flip(hip.noisy_position)];
        y2 = [hip.noisy_position; zeros(length(hip.noisy_position),1)];
        filter_y1 = conv(y1,h)*3.6;
        filter_y2 = conv(y2,h)*3.6;
        figure(15);
        plot(1:N,filter_y1(31:31+N-1),'LineWidth',2,Color='y')
        hold on;
        plot(1:N,filter_y2(31:31+N-1),Color='r')
        axis([0 600 -100 220])
        title('output of filtering the signal y1 and y2');
        legend('filter y1','filter y2','FontSize', 15)
        % 
               

                           
    end
   


funs.gen_filter = @gen_filter;


% This file will return a structure with handles to the functions you have
% implemented. You can call them if you wish, for example:
% funs = student_sols();
% some_output = funs.some_function(some_input);

end

