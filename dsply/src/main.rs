use glib::clone;
use chrono::{DateTime,Local};
use gtk4::glib;
use gtk4::prelude::*;
use std::path::Path;
use gdk4;
use gtk4::gio;
use std::process::ExitCode;
use gdk_pixbuf::Pixbuf;

fn texture_from_filename(file: String) -> Option<gdk4::Texture>{
	let pixbuf = match Pixbuf::from_file_at_scale(&file,400,400,true) {
		Ok(res) => res,
		Err(err) => {println!("error loading {}: {}",&file,err);return None;}
	};
	Some(gdk4::Texture::for_pixbuf(&pixbuf))
}
fn file_info_string(file: String) -> Option<String>{
	//--- load the file to get size ---
	let pixbuf = match Pixbuf::from_file(&file) {
		Ok(res) => res,
		Err(err) => {println!("error loading {}: {}",&file,err);return None;}
	};
	let width = pixbuf.width();
	let height = pixbuf.height();
	//--- create path ---
	let file_path = Path::new(&file);
	let metadata = file_path.metadata();
	let creation_date_str = match metadata {
		Ok(data) => match data.created() {
			Ok(date) => format!("{}",date.clone().into() as DateTime<Local>),
			Err(error) => "Unknown creation"
		},
		Err(e) => "Unknown creation"
	};
	//--- construct string ---
	Some(format!("{}x{}\n{}\n{}",width,height,creation_date_str,"None"))
}

fn main() -> ExitCode {
	let files = std::env::args().collect::<Vec<String>>()[1..].to_vec();
	if files.len() < 1{
		println!("Please provide at least one file");
		return ExitCode::from(99);
	}
	for file in files{
		println!("{}",file);
		let path = Path::new(&file);
		if !path.exists(){
			println!("Error: {} does not exist",file);
			return ExitCode::from(2);
		}
	}
	let app = gtk4::Application::builder()
		.application_id("com.github.rufus173.dsply")
		.build();
	app.connect_activate(on_activate);
	app.run_with_args(&Vec::<String>::new());
	ExitCode::from(0)
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
	let file_list = std::env::args().collect::<Vec<String>>()[1..].to_vec();
	let current_image: i32 = 0;
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
	grid.attach(&close_button,1,1,1,1);
	//--- image name/path label ---
	let image_name_label = gtk4::Label::new(Some(file_list[0].as_str()));
	grid.attach(&image_name_label,0,0,2,1);
	//--- image display ---
	let image_display = gtk4::Picture::new();
	image_display.set_content_fit(gtk4::ContentFit::ScaleDown);
	image_display.set_hexpand(false);
	image_display.set_vexpand(false);
	image_display.set_paintable(texture_from_filename(file_list[0].clone()).as_ref());
	grid.attach(&image_display,0,1,1,4);
	//--- info panel ---
	let info_panel = gtk4::Label::new(file_info_string(file_list[0].clone()).as_deref());
	grid.attach(&info_panel,1,4,1,1);
	//--- previous button ---
	let previous_button = gtk4::Button::with_label("Previous");
	grid.attach(&previous_button,1,3,1,1);
	previous_button.connect_clicked(move |previous_button|{
		let parameter = -1;
		previous_button.activate_action("win.change-image",Some(&parameter.to_variant())).expect("action does not exist");
	});
	//--- next button ---
	let next_button = gtk4::Button::with_label("Next");
	grid.attach(&next_button,1,2,1,1);
	next_button.connect_clicked(move |next_button|{
		let parameter = 1;
		next_button.activate_action("win.change-image",Some(&parameter.to_variant())).expect("action does not exist");
	});
	//====== actions ======
	let action_change_image = gio::ActionEntry::builder("change-image")
		.parameter_type(Some(&i32::static_variant_type()))
		.state(current_image.to_variant())
		.activate(move |_, action, parameter|{
			//--- extract variables from action ---
			let parameter = parameter
				.expect("Could not fetch parameter")
				.get::<i32>()
				.expect("variant not of type u32");
			let mut current_image = action
				.state()
				.expect("could not get state")
				.get::<i32>()
				.expect("variant not of type i32");
			
			current_image += parameter;
			if current_image >= 0 && current_image < (file_list.len() as i32){
				//------ update the widgets ------
				image_name_label.set_text(file_list[current_image as usize].as_str());
				image_display.set_paintable(texture_from_filename(file_list[current_image as usize].clone()).as_ref());
				println!("{}",current_image);
				match file_info_string(file_list[current_image as usize].clone()).as_deref(){
					Some(info) => info_panel.set_text(info),
					None => info_panel.set_text("")
				};
				action.set_state(&current_image.to_variant())
			}
		})
		.build();
	window.add_action_entries([action_change_image]);
	window.present();
}
