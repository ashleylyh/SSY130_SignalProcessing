%NO_PFILE
function plot_constallation(tx, rx)
%PLOT_CONSTALLATION Create a constellation plot of the transmitted and
%recieved data vectors in the current figure.

clf;
c = linspace(0, length(rx), length(rx));
sz = linspace(10, 60, length(rx));
scatter(real(rx), imag(rx), sz, c);
h_c = colorbar();
title(h_c, 'symbol index');
hold on;
scatter(real(tx), imag(tx), 15, 'filled', 'red');
xlabel('Re\{s\}');
ylabel('Im\{s\}');
legend('Rx', 'Tx', 'Location', 'Best');

grid on;
axis equal;
daspect([1,1,1]);

end