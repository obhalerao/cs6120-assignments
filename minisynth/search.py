if __name__ == '__main__':
    for mul_guess in range(2 ** 8):
        for shift_guess in range(8):
            counterexample_count = 0
            for i in range(256):
                if i // 7 == (i * mul_guess)