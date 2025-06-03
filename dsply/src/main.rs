use glib::clone;
use gtk4::glib;
use gtk4::prelude::*;
use std::path::Path;
use gtk4::gdk_pixbuf::Pixbuf;

pub struct Images {
	pixbufs: Vec<gtk4::gdk_pixbuf::Pixbuf>,
	image_names: Vec<String>,
}
impl Images {
	fn new(files: Vec<String>) -> Option<Images>{
		let mut images = Images {
			pixbufs: Vec::new(),
			image_names: files.clone(),
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
/*
artist's rendition of the finished product
         based off image size          constant
  <--------------------------------> <---------->
+-------------------------------------------------+
| +--------------------------------+ +----------+ |
| | path/to/image/file             | |  close   | |
| +--------------------------------+ +----------+ |
| +--------------------------------+ +----------+ |
| |                                | | previous | |
| |                                | +----------+ |
| |                                | +----------+ |
| |              (__)              | |   next   | |
| |              (oo)              | +----------+ |
| |        /------\/               | +----------+ |
| |       / |    ||                | | 180x200  | |
| |      *  /\---/\                | | mod. 5/8 | |
| |         ~~   ~~                | | auth:    | |
| |                                | | john s.  | |
| |                                | | leon s.  | |
| |                                | |          | |
| |                                | | image 6  | |
| |                                | | 5<- ->7  | |
| |                                | | 10 more  | |
| +--------------------------------+ +----------+ |
+-------------------------------------------------+
*/

fn on_activate(application: &gtk4::Application){
	//====== load the images ======
	let file_list: Vec<String> = std::env::args().collect();
	let images = Images::new(file_list[1..].to_vec());
	//====== build the gui ======
	let window = gtk4::ApplicationWindow::builder()
		.application(application)
		.title("dsply")
		.resizable(false)
		.build();
	let grid = gtk4::Grid::new();
	window.set_child(Some(&grid));
	//--- close button ---
	let close_button = gtk4::Button::with_label("Close");
	close_button.connect_clicked(
		clone!(#[strong] window, move |_|{
			window.close();
		})
	);
	grid.attach(&close_button,1,0,1,1);
	//--- image name/path label ---
	let image_name_label = gtk4::Label::new(Some("/a/b/img.png"));
	grid.attach(&image_name_label,0,0,1,1);
	//--- image display ---
	let image_display = gtk4::Image::new();
	grid.attach(&image_display,0,0,1,3);
	//--- previous button ---
	let previous_button = gtk4::Button::with_label("Previous");
	grid.attach(&previous_button,1,1,1,1);
	//--- next button ---
	let next_button = gtk4::Button::with_label("Next");
	grid.attach(&next_button,1,2,1,1);
	//--- info panel ---
	let info_panel = gtk4::Label::new(Some("size\nother\ndate of creation"));
	grid.attach(&info_panel,1,3,1,1);
	window.present();
}
