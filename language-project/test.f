C<8, 8>(i, j) = A<8>(i);

C<4, 8>(i, j) = A<4, 8, 4>(i, j, i);

C<20, 16>(i, j) = A<20, 8>(i, k) * B<8, 16>(k, j);

C<8, 4, 7>(n, k, l) = A<8, 16, 9>(n, c, l+r) * W<4, 16, 3>(k, c, r);

C<8, 4, 7>(n, k, l) = A<8, 16, 23>(n, c, l*3+r*2) * W<4, 16, 3>(k, c, r);