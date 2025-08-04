fn collatz(mut n: u64) -> u64 {
    let mut i = 0;
    while n > 1 {
        n = if n % 2 == 0 {
            n / 2
        } else {
            n * 3 + 1
        };
        i += 1;
    }
    i
}

fn max_len(lim: u64) -> u64 {
    let mut max = 0;
    for i in 1..lim {
        let n = collatz(i);
        if n > max {
            max = n;
        }
    }
    max
}

fn main() {
    let max = max_len(1_000_000);
    println!("{max}");
}
