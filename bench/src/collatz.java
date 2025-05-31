class Collatz {

	private static int collatz(long n) {
		int i = 0;
		while (n > 1) {
			i++;
			if (n % 2 == 0) {
				n = n / 2;
			} else {
				n = 3 * n + 1;
			}
		}
		return i;
	}

	private static int maxlen(long n) {
		int max = 0;
		for (int i = 1; i < n; i++) {
			int m = collatz(i);
			if (m > max) {
				max = m;
			}
		}
		return max;
	}

	public static void main(String[] args) {
		System.out.println(maxlen(1000000));
	}

}
