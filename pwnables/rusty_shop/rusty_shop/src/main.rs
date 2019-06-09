#![allow(dead_code)]

use std::io;
use std::io::Write;
use std::thread;
use libc::system;

struct YorkshireCanary;
struct NorwichCanary;

trait NamedCanary {
    fn get_name(&self) -> String;
}

impl NamedCanary for YorkshireCanary {
    fn get_name(&self) -> String {
        return String::from("Yorkshire");
    }
}

impl NamedCanary for NorwichCanary {
    fn get_name(&self) -> String {
        return String::from("Norwich");
    }
}

static FUNC: (fn()->()) = win;
static CMD: [i8; 26] = [0x63,0x61,0x74,0x20,0x2f,0x68,0x6f,0x6d,0x65,0x2f,0x72,0x75,0x73,0x74,0x79,0x5f,0x73,0x68,0x6f,0x70,0x2f,0x66,0x6c,0x61,0x67,0x00]; // cat /home/rusty_shop/flag\0

struct Item {
    name: String,
    description: String,
    price: f32,
}

struct BasketItem {
    item: *const Item,
    number: usize,
}

fn print_menu() {
    println!("1. Create item");
    println!("2. Delete item");
    println!("3. Show item");
    println!("4. Add item to basket");
    println!("5. Show basket");
    println!("6. Check out");
}

fn create_item(items: &mut Vec<Item>) {
    print!("Name: ");
    io::stdout().flush().unwrap();
    let mut title = String::new();
    io::stdin().read_line(&mut title).unwrap();
    title.pop();
    
    print!("Description: ");
    io::stdout().flush().unwrap();
    let mut desc  = String::new();
    io::stdin().read_line(&mut desc).unwrap();
    desc.pop();

    print!("Price: ");
    io::stdout().flush().unwrap();
    let mut price = String::new();
    io::stdin().read_line(&mut price).unwrap();
    price.pop();
    let num_price = price.parse::<f32>().unwrap();

    items.push(Item {
        name: title,
        description: desc,
        price: num_price,
    });
}

fn show_item(items: &Vec<Item>) {
    print!("Item number: ");
    io::stdout().flush().unwrap();
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();
    input.pop();
    let idx = input.parse::<usize>().unwrap();

    let i = &items[idx];
    
    println!("Name: {}", i.name);
}


fn add_item(items: &Vec<Item>, basket: &mut Vec<BasketItem>) {
    let mut input = String::new();

    print!("Item index to add: ");
    io::stdout().flush().unwrap();
    io::stdin().read_line(&mut input).unwrap();
    input.pop();
    let idx = input.parse::<usize>().unwrap();

    print!("Count: ");
    input.clear();
    io::stdout().flush().unwrap();
    io::stdin().read_line(&mut input).unwrap();
    input.pop();
    let count = input.parse::<usize>().unwrap();

    basket.push(BasketItem {
        item: &items[idx] as *const Item,
        number: count, 
    });
}

fn check_out(basket: &Vec<BasketItem>) {
    let mut invoice = Vec::new();
    for i in basket.iter() {
        let s: String;
        unsafe {
            //s = format!("{}  {}", (*i.item).name, (*i.item).price);
            s = format!("{}", (*i.item).name);
        }

        invoice.push(s.repeat(i.number));
    }	

    println!("Invoice\n--------");
    for item in invoice {
        println!("{}", item);
    }
}

fn detect_hacking() {
    thread::spawn(move || {
        loop {
            let mut canaries: Vec<Box<NamedCanary>> = Vec::with_capacity(2);
            canaries.push(Box::new(YorkshireCanary));
            canaries.push(Box::new(NorwichCanary));
            
            if canaries[0].get_name() != String::from("Yorkshire") 
                || canaries[1].get_name() != String::from("Norwich") {
                println!("Hacking detected!");
            }

        }
    });

}

fn main() {
    detect_hacking();
    let main_thread = run_shop();
    main_thread.join().unwrap();
    std::process::exit(0);
}

fn run_shop() -> std::thread::JoinHandle<()> {
    return thread::spawn(move || {
        let mut items: Vec<Item> = Vec::with_capacity(4);
        let mut basket: Vec<BasketItem> = Vec::new();

        items.push(Item {
            name: String::from("Apple"),
            description: String::from("Succulent Granny Smith's green apple"),
            price: 0.59,
        });
        loop {
            print_menu();
            let mut input = String::new();
            io::stdin().read_line(&mut input).unwrap();
            input.pop();
            let choice = input.parse::<i32>().unwrap();
            match choice {
                1 => create_item(&mut items),
                2 => println!("TODO"),
                3 => show_item(&items),
                4 => add_item(&items, &mut basket),
                5 => println!("TODO"),
                6 => check_out(&basket),
                _ => println!("Invalid choice.")
            }
        }   
    });
}

fn win() {
    unsafe {
        system(&CMD as *const i8);
    }
}
