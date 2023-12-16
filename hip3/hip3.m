%NO_PFILE
% HIP3

% Perform the following steps:
%   1) In student_sols.m, update the student_id variable as described
%   there.
%
%   2) In student_sols.m, complete all the partially-complete functions.
%   (Indicated by a '%TODO: ...' comment). Note that all the functions you
%   need to complete are located in student_sols.m (and only in
%   student_sols.m). You can test these functions by running this file,
%   which will apply a self-test to your functions. When all functions pass
%   the self-test, a unique password will be printed in the terminal. Be
%   sure to include this password in your submission.
%
%   3) Now that the functions in student_sols.m are completed, continue
%   working with this file. Notably, your finished functions will be used
%   to evaluate the behavior of the assignment.
%
% -------------------------------------------------------------------------
%                    Note on function handles
% -------------------------------------------------------------------------
% In this file, we will make use of function handles. A function handle is
% a variable that refers to a function. For example:
%
% x = @plot
%
% assigns a handle to the plot function to the variable x. This allows to
% for example do something like
%
% x(sin(linspace(0,2*pi)))
%
% to call the plot function. Usefully for you, there exist function handles
% to all the functions you've written in student_sols.m. See below for
% exactly how to call them in this assignment.
%
% -------------------------------------------------------------------------
%                    Final notes
% -------------------------------------------------------------------------
%
% The apply_tests() function will set the random-number generator to a
% fixed seed (based on the student_id parameter). This means that repeated
% calls to functions that use randomness will return identical values. This
% is in fact a "good thing" as it means your code is repeatable. If you
% want to perform multiple tests you will need to call your functions
% several times after the apply_tests() function rather than re-running
% this entire file.
%
% Note on debugging: if you wish to debug your solution (e.g. using a
% breakpoint in student_sols.m), comment out the line where the apply_tests
% function is called in the hand-in/project script. If you do not do this
% then you'll end up debugging your function when it is called during the
% self-test routine, which is probably not what you want. (Among other
% things, you won't be able to control the input to your functions).
%
% Files with a .p extension are intentionally obfusticated (they cannot
% easily be read). These files contain the solutions to the tasks you are
% to solve (and are used in order to self-test your code). Though it is
% theoretically possible to break into them and extract the solutions,
% doing this will take you *much* longer than just solving the posed tasks
% =)

% Do some cleanup
clc
clear variables
format short eng

% Perform all self-tests of functions in student_sol.m
apply_tests();

% Load student-written functions
funs = student_sols();

% Set up ground-truth motion
x = 0:0.01:9.99;
y = sin(0.5*x);
Y = [x;y];
Z = Y + 0.1*randn(size(Y));

% Plot input motion
figure(1);
plot(Y(1,:), Y(2,:));
xlabel('x');
ylabel('y');
title('Noise-free position');

figure(2);
scatter(Z(1,:), Z(2,:));
xlabel('x');
ylabel('y');
title('Measured position');

% Set up A, C, Q, R, x0, P0 here

% Call your fancy kalman filter using the syntax
% [Xfilt, Pp] = funs.kalm_filt(Z,A,C,Q,R,x0,P0);

% Plot input motion
figure(3);
% plot(Y(1,:), Y(2,:));
plot(Y(1,:),Y(2,:),'x');
hold on
scatter(Z(1,:),Z(2,:),'x');
xlabel('x');
ylabel('y');
title('target tracking');
legend('Position without noise', 'measured noisy position')
 
% figure(2);
% scatter(Z(1,:), Z(2,:));
% plot(Z(1,:),Z(2,:),'x')
% xlabel('x');
% ylabel('y');
% title('Measured position');

% Set up A, C, Q, R, x0, P0 here

% Call your fancy kalman filter using the syntax
 T = 0.01;   % s
A = [1 T 0 0;
     0 1 0 0;
     0 0 1 T;
     0 0 0 1];
C = [1 0 0 0;
     0 0 1 0];

 
%% Sensor Calibration 

e = [1 0; 0 1]*(Z-Y);

pdX = fitdist(e(1,:)','Normal');
pdY = fitdist(e(2,:)','Normal');

R = blkdiag(pdX.sigma^2, pdY.sigma^2);      % Measurement noise covariance

alpha = 1e-3;
Q = blkdiag(0, 1, 0, 1)*alpha;              % Process noise covariance

%% Prediction

P0 = 1e6*eye(size(A));
x0 = [0 0 0 0]';
[Xfilt, Pp] = funs.kalm_filt(Z,A,C,Q,R,x0,P0);


% %% Q4 Covariance analysis
% figure('Color','white')
% [S,AX,BigAx,H,HAx] = plotmatrix(e');
% 
% x_values = linspace(HAx(1).XLim(1),HAx(1).XLim(2),100);
% pdXv = pdf(pdX,x_values);
% pdXv = pdXv * 0.9*HAx(1).YLim(2)/max(pdXv);
% hold(HAx(1),'on');
% plot(HAx(1),x_values,pdXv,'LineWidth',2)
% xlabel 'x', ylabel 'y', title 'Covariance analysis'
% 
% x_values = linspace(HAx(2).XLim(1),HAx(2).XLim(2),100);
% pdYv = pdf(pdY,x_values);
% pdYv = pdYv * 0.9*HAx(2).YLim(2)/max(pdYv);
% hold(HAx(2),'on');
% plot(HAx(2),x_values,pdYv,'LineWidth',2)
% 
% AX(1).YLabel.String = 'Error X';
% AX(2).YLabel.String = 'Error Y';
% AX(2).XLabel.String = 'Error X';
% AX(4).XLabel.String = 'Error Y';
% fancyplot.savefig('covar');
% 
%% Q4 Kalman Filter plot plot for  traking possion with noise an without noise  
figure('Color','white'); hold on; grid on;
scatter(Z(1,:), Z(2,:), '<', 'MarkerEdgeColor',0.8*[1 1 1]);
p1 = plot(Xfilt(1,:), Xfilt(3,:), 'LineWidth',3);
% scatter(Xfilt(1,:), Xfilt(3,:));
p2 = plot(Y(1,:), Y(2,:),'-','LineWidth',3, 'Color',fancyplot.getColor(4,.8));
xlabel 'x', ylabel 'y', title 'Target Tracking'
legend({'Measured position','Kalman-Filter','Noise-free position'});
fancyplot.savefig(['target-tracking-',num2str(alpha,'%10.0e\n')]);


h_euler = [1/T , -1/T];

%% Q4 Kalman Filter plot - VELOCITY
t = (0:length(Z)-1) ./T;
figure('Color','white'); hold on; grid on;
subplot(2,1,1); hold on; grid on;
plot(t, Xfilt(2,:), 'LineWidth',2);
plot(t(1:end-1), conv(Y(1,:), h_euler, 'valid'), 'LineWidth',2);
xlabel 'time [s]', ylabel 'Velocity x', title 'Velocity Tracking'
ylim([0.8,1.2]);
legend({'Kalman-Filter','Noise-free velocity'});
subplot(2,1,2); hold on; grid on;
plot(t, Xfilt(4,:), 'LineWidth',2);
plot(t(1:end-1), conv(Y(2,:), h_euler, 'valid'), 'LineWidth',2);
xlabel 'time [s]', ylabel 'Velocity y'
ylim([-0.5,0.2]);
legend({'Kalman-Filter','Noise-free velocity'});
fancyplot.savefig('velocity-tracking');
