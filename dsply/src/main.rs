use glib::clone;
use gtk4::glib;
use gtk4::prelude::*;
use std::path::Path;
use gdk4;
use gio;
use std::rc::Rc;

pub struct Images {
	textures: Vec<gdk4::Texture>,
	image_names: Vec<String>,
	current_image: usize,
}
impl Images {
	fn new(files: Vec<String>) -> Option<Images>{
		let mut images = Images {
			textures: Vec::new(),
			image_names: Vec::new(),
			current_image: 0,
		};
		for file in files{
			println!("{}",file);
			let path = Path::new(&file);
			if !path.exists(){
				println!("Error: {} does not exist",file);
				continue;
			}
			let gfile = gio::File::for_path(&file);
			images.textures.push(match gdk4::Texture::from_file(&gfile){
				Err(e) => {println!("{}",e); continue;},
				Ok(pixbuf) => pixbuf,
			});
			images.image_names.push(file);
		}
		Some(images)
	}
	fn next_image(&mut self) -> Result<(),&str>{
		if self.current_image >= self.textures.len(){
			return Err("image out of range");
		}
		self.current_image += 1;
		Ok(())
	}
	fn previous_image(&mut self) -> Result<(),&str>{
		if self.current_image < 1{
			return Err("image out of range");
		}
		self.current_image -= 1;
		Ok(())
	}
	fn get_texture(&self) -> Option<&gdk4::Texture>{
		if self.textures.len() == 0{
			return None;
		}
		Some(&self.textures[self.current_image])
	}
	fn get_image_name(&self) -> Option<String>{
		if self.textures.len() == 0{
			return None;
		}
		Some(self.image_names[self.current_image].clone())
	}
}

fn main(){
	if std::env::args().collect::<Vec<String>>().len() < 2{
		println!("Please provide at least one file");
		return;
	}
	let app = gtk4::Application::builder()
		.application_id("com.github.rufus173.dsply")
		.build();
	app.connect_activate(on_activate);
	app.run_with_args(&Vec::<String>::new());
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
	let mut images = match Images::new(file_list[1..].to_vec()){
		Some(images) => images,
		None => panic!("Could not initialise images"),
	};
	//====== actions ======
	let action_next = ActionEntry::builder()
		.activate(
		);
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
	let image_name_label = gtk4::Label::new(images_weak.upgrade().unwrap().get_image_name().as_deref());
	grid.attach(&image_name_label,0,0,1,1);
	//--- image display ---
	let image_display = gtk4::Picture::new();
	image_display.set_paintable(images_weak.upgrade().unwrap().get_texture());
	grid.attach(&image_display,0,0,1,4);
	//--- info panel ---
	let info_panel = gtk4::Label::new(Some("size\nother\ndate of creation"));
	grid.attach(&info_panel,1,3,1,1);
	//--- previous button ---
	let previous_button = gtk4::Button::with_label("Previous");
	grid.attach(&previous_button,1,1,1,1);
	//--- next button ---
	let next_button = gtk4::Button::with_label("Next");
	grid.attach(&next_button,1,2,1,1);
	window.present();
}
