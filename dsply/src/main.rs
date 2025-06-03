use glib::clone;
use gtk4::glib;
use gtk4::prelude::*;
use std::path::Path;
use gtk4::gdk_pixbuf::Pixbuf;

pub struct Images {
	pixbufs: Vec<gtk4::gdk_pixbuf::Pixbuf>,
}
impl Images {
	fn new(files: Vec<String>) -> Option<Images>{
		let mut images = Images {
			pixbufs: Vec::new(),
		};
		for file in files{
			println!("{}",file);
			let path = Path::new(&file);
			if !path.exists(){
				println!("Error: {} does not exist",file);
				continue;
			}
			images.pixbufs.push(match Pixbuf::from_file(&file){
				Err(e) => {println!("{}",e); continue;},
				Ok(pixbuf) => pixbuf,
			});
		}
		Some(images)
	}
}

fn main() {
	let app = gtk4::Application::builder()
		.application_id("com.github.rufus173.dsply")
		.build();
	app.connect_activate(on_activate);
	app.run();
}

fn on_activate(application: &gtk4::Application){
	//====== load the images ======
	let file_list: Vec<String> = std::env::args().collect();
	let images = Images::new(file_list[1..].to_vec());
	//====== build the gui ======
	let window = gtk4::ApplicationWindow::builder()
		.application(application)
		.title("dsply")
		.resizable(false)
		.default_width(500)
		.default_height(500)
		.build();
	let grid = gtk4::Grid::new();
	window.set_child(Some(&grid));
	let close_button = gtk4::Button::with_label("Close");
	close_button.connect_clicked(
		clone!(#[strong] window, move |_|{
			window.close();
		})
	);
	GridExt::attach(&grid,&close_button,0,0,1,1);
	let image_display = gtk4::Image::new();
	window.present();
}
