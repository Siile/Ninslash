import copy
from datatypes import *

class Sound(Struct):
	def __init__(self, filename=""):
		Struct.__init__(self, "CDataSound")
		self.id = Int(0)
		self.filename = String(filename)

class SoundSet(Struct):
	def __init__(self, name="", files=[]):
		Struct.__init__(self, "CDataSoundset") 
		self.name = String(name)
		self.sounds = Array(Sound())
		self.last = Int(-1)
		for name in files:
			self.sounds.Add(Sound(name)) 

class Image(Struct):
	def __init__(self, name="", filename=""):
		Struct.__init__(self, "CDataImage")
		self.name = String(name)
		self.filename = String(filename)
		self.id = Int(-1)

class SpriteSet(Struct):
	def __init__(self, name="", image=None, gridx=0, gridy=0):
		Struct.__init__(self, "CDataSpriteset")
		self.image = Pointer(Image, image) # TODO
		self.gridx = Int(gridx)
		self.gridy = Int(gridy)

class Sprite(Struct):
	def __init__(self, name="", Set=None, x=0, y=0, w=0, h=0):
		Struct.__init__(self, "CDataSprite")
		self.name = String(name)
		self.set = Pointer(SpriteSet, Set) # TODO
		self.x = Int(x)
		self.y = Int(y)
		self.w = Int(w)
		self.h = Int(h)

class Pickup(Struct):
	def __init__(self, name="", respawntime=15, spawndelay=0):
		Struct.__init__(self, "CDataPickupspec")
		self.name = String(name)
		self.respawntime = Int(respawntime)
		self.spawndelay = Int(spawndelay)

class WeaponSpec(Struct):
	def __init__(self, container=None, name=""):
		Struct.__init__(self, "CDataWeaponspec")
		self.name = String(name)
		self.sprite_body = Pointer(Sprite, Sprite())
		self.sprite_pickup = Pointer(Sprite, Sprite())
		self.sprite_cursor = Pointer(Sprite, Sprite())
		self.sprite_proj = Pointer(Sprite, Sprite())
		self.sprite_muzzles = Array(Pointer(Sprite, Sprite()))
		self.visual_size = Int(96)

		self.firedelay = Int(500)
		self.maxammo = Int(10)
		self.ammoregentime = Int(0)
		self.damage = Int(1)

		self.offsetx = Float(0)
		self.offsety = Float(0)
		self.muzzleoffsetx = Float(0)
		self.muzzleoffsety = Float(0)
		self.muzzleduration = Float(5)

		# dig out sprites if we have a container
		if container:
			for sprite in container.sprites.items:
				if sprite.name.value == "weapon_"+name+"_body": self.sprite_body.Set(sprite)
				elif sprite.name.value == "weapon_"+name+"_pickup": self.sprite_pickup.Set(sprite)
				elif sprite.name.value == "weapon_"+name+"_cursor": self.sprite_cursor.Set(sprite)
				elif sprite.name.value == "weapon_"+name+"_proj": self.sprite_proj.Set(sprite)
				elif "weapon_"+name+"_muzzle" in sprite.name.value:
					self.sprite_muzzles.Add(Pointer(Sprite, sprite))

class Weapon_Tool(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecTool")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		
class Weapon_Hammer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecHammer")
		self.base = Pointer(WeaponSpec, WeaponSpec())

class Weapon_Chainsaw(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecChainsaw")
		self.base = Pointer(WeaponSpec, WeaponSpec())

class Weapon_Gun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.lifetime = Float(2.0)

class Weapon_Shotgun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecShotgun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.speeddiff = Float(0.8)
		self.lifetime = Float(0.25)

class Weapon_Grenade(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGrenade")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(7.0)
		self.speed = Float(1000)
		self.lifetime = Float(2.0)

class Weapon_Rifle(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecRifle")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)
		
class Weapon_Electric(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecElectric")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)

class Weapon_Laser(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecLaser")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)
		self.bounce_delay = Int(150)
		self.bounce_num = Int(1)
		self.bounce_cost = Float(0)

class Weapon_Flamer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecFlamer")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)

		

class Weapons(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecs")
		self.tool = Weapon_Tool()
		self.hammer = Weapon_Hammer()
		self.chainsaw = Weapon_Chainsaw()
		self.gun = Weapon_Gun()
		self.shotgun = Weapon_Shotgun()
		self.grenade = Weapon_Grenade()
		self.rifle = Weapon_Rifle()
		self.electric = Weapon_Electric()
		self.laser = Weapon_Laser()
		self.flamer = Weapon_Flamer()
		self.id = Array(WeaponSpec())

class DataContainer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataContainer")
		self.sounds = Array(SoundSet())
		self.images = Array(Image())
		self.pickups = Array(Pickup())
		self.spritesets = Array(SpriteSet())
		self.sprites = Array(Sprite())
		self.weapons = Weapons()

def FileList(format, num):
	return [format%(x+1) for x in range(0,num)]

container = DataContainer()
container.sounds.Add(SoundSet("gun_fire", FileList("audio/wp_gun_fire-%02d.wv", 3)))
container.sounds.Add(SoundSet("shotgun_fire", FileList("audio/wp_shotty_fire-%02d.wv", 3)))

container.sounds.Add(SoundSet("electro_fire", FileList("audio/wp_electro_fire-%02d.wv", 3)))

container.sounds.Add(SoundSet("grenade_fire", FileList("audio/wp_flump_launch-%02d.wv", 3)))
container.sounds.Add(SoundSet("tool_fire", FileList("audio/wp_hammer_swing-%02d.wv", 3)))
container.sounds.Add(SoundSet("tool_hit", FileList("audio/wp_hammer_hit-%02d.wv", 3)))
container.sounds.Add(SoundSet("hammer_fire", FileList("audio/wp_sword_attack-%02d.wv", 3)))
container.sounds.Add(SoundSet("hammer_hit", FileList("audio/wp_sword_hit-%02d.wv", 3)))
container.sounds.Add(SoundSet("chainsaw_idle", ["audio/wp_chainsaw_idle.wv"]))
container.sounds.Add(SoundSet("chainsaw_fire", ["audio/wp_chainsaw_fire.wv"]))
container.sounds.Add(SoundSet("grenade_explode", FileList("audio/wp_flump_explo-%02d.wv", 3)))
container.sounds.Add(SoundSet("rifle_fire", FileList("audio/wp_rifle_fire-%02d.wv", 3)))
container.sounds.Add(SoundSet("laser_fire", FileList("audio/wp_laser_fire-%02d.wv", 3)))
container.sounds.Add(SoundSet("laser_bounce", FileList("audio/wp_rifle_bnce-%02d.wv", 3)))
container.sounds.Add(SoundSet("weapon_switch", FileList("audio/wp_switch-%02d.wv", 3)))

container.sounds.Add(SoundSet("player_pain_short", FileList("audio/vo_teefault_pain_short-%02d.wv", 12)))
container.sounds.Add(SoundSet("player_pain_long", FileList("audio/vo_teefault_pain_long-%02d.wv", 2)))

container.sounds.Add(SoundSet("body_land", FileList("audio/foley_land-%02d.wv", 4)))
container.sounds.Add(SoundSet("player_airjump", FileList("audio/foley_dbljump-%02d.wv", 3)))
container.sounds.Add(SoundSet("player_jump", FileList("audio/foley_foot_left-%02d.wv", 4) + FileList("audio/foley_foot_right-%02d.wv", 4)))
container.sounds.Add(SoundSet("player_die", FileList("audio/foley_body_splat-%02d.wv", 3)))
container.sounds.Add(SoundSet("player_spawn", FileList("audio/vo_teefault_spawn-%02d.wv", 7)))
container.sounds.Add(SoundSet("player_skid", FileList("audio/sfx_skid-%02d.wv", 4)))
container.sounds.Add(SoundSet("tee_cry", FileList("audio/vo_teefault_cry-%02d.wv", 2)))

container.sounds.Add(SoundSet("hook_loop", FileList("audio/hook_loop-%02d.wv", 2)))

container.sounds.Add(SoundSet("hook_attach_ground", FileList("audio/hook_attach-%02d.wv", 3)))
container.sounds.Add(SoundSet("hook_attach_player", FileList("audio/foley_body_impact-%02d.wv", 3)))
container.sounds.Add(SoundSet("hook_noattach", FileList("audio/hook_noattach-%02d.wv", 2)))
container.sounds.Add(SoundSet("pickup_health", FileList("audio/sfx_pickup_hrt-%02d.wv", 2)))
container.sounds.Add(SoundSet("pickup_armor", FileList("audio/sfx_pickup_arm-%02d.wv", 4)))

container.sounds.Add(SoundSet("pickup_grenade", ["audio/sfx_pickup_launcher.wv"]))
container.sounds.Add(SoundSet("pickup_shotgun", ["audio/sfx_pickup_sg.wv"]))
container.sounds.Add(SoundSet("weapon_spawn", FileList("audio/sfx_spawn_wpn-%02d.wv", 3)))
container.sounds.Add(SoundSet("weapon_noammo", FileList("audio/wp_noammo-%02d.wv", 5)))

container.sounds.Add(SoundSet("hit", FileList("audio/sfx_hit_weak-%02d.wv", 2)))


container.sounds.Add(SoundSet("ui_pick", ["audio/ui_pick.wv"]))
container.sounds.Add(SoundSet("ui_negative", ["audio/ui_negative.wv"]))
container.sounds.Add(SoundSet("ui_positive", ["audio/ui_positive.wv"]))

container.sounds.Add(SoundSet("chat_server", ["audio/sfx_msg-server.wv"]))
container.sounds.Add(SoundSet("chat_client", ["audio/sfx_msg-client.wv"]))
container.sounds.Add(SoundSet("chat_highlight", ["audio/sfx_msg-highlight.wv"]))
container.sounds.Add(SoundSet("ctf_drop", ["audio/sfx_ctf_drop.wv"]))
container.sounds.Add(SoundSet("ctf_return", ["audio/sfx_ctf_rtn.wv"]))
container.sounds.Add(SoundSet("ctf_grab_pl", ["audio/sfx_ctf_grab_pl.wv"]))
container.sounds.Add(SoundSet("ctf_grab_en", ["audio/sfx_ctf_grab_en.wv"]))
container.sounds.Add(SoundSet("ctf_capture", ["audio/sfx_ctf_cap_pl.wv"]))

container.sounds.Add(SoundSet("metal_hit", FileList("audio/metal_hit-%02d.wv", 3)))

container.sounds.Add(SoundSet("jetpack", ["audio/jetpack.wv"]))
container.sounds.Add(SoundSet("jetpack1", ["audio/jetpack1.wv"]))
container.sounds.Add(SoundSet("jetpack2", ["audio/jetpack2.wv"]))

container.sounds.Add(SoundSet("deathray", ["audio/deathray.wv"]))
container.sounds.Add(SoundSet("electrodeath", ["audio/electrodeath.wv"]))
container.sounds.Add(SoundSet("spawn", ["audio/spawn.wv"]))

container.sounds.Add(SoundSet("shield_hit", FileList("audio/shield_hit-%02d.wv", 3)))

container.sounds.Add(SoundSet("item_invisibility", ["audio/item_invisibility.wv"]))
container.sounds.Add(SoundSet("item_shield", ["audio/item_shield.wv"]))
container.sounds.Add(SoundSet("item_heal", ["audio/item_heal.wv"]))
container.sounds.Add(SoundSet("item_rage", ["audio/item_rage.wv"]))
container.sounds.Add(SoundSet("item_fuel", ["audio/item_fuel.wv"]))

container.sounds.Add(SoundSet("electromine", ["audio/electromine.wv"]))

container.sounds.Add(SoundSet("menu", ["audio/music_menu.wv"]))



image_null = Image("null", "")
image_particles = Image("particles", "particles.png")
image_gore = Image("gore", "gore.png")
image_splatter = Image("splatter", "splatter.png")
image_game = Image("game", "game.png")
image_flag = Image("flag", "flag.png")
image_weapons = Image("weapons", "weapons.png")
image_deathtypes = Image("deathtypes", "deathtypes.png")
image_items = Image("items", "items.png")
image_itemnumbers = Image("itemnumbers", "itemnumbers.png")
image_sword = Image("sword", "sword.png")
image_hand = Image("hand", "hand.png")
image_bodies = Image("bodies", "bodies.png")
image_pickups = Image("pickups", "pickups.png")
image_buildings = Image("buildings", "buildings.png")
image_browseicons = Image("browseicons", "browse_icons.png")
image_emoticons = Image("emoticons", "emoticons.png")
image_demobuttons = Image("demobuttons", "demo_buttons.png")
image_fileicons = Image("fileicons", "file_icons.png")
image_guibuttons = Image("guibuttons", "gui_buttons.png")
image_guiicons = Image("guiicons", "gui_icons.png")
image_lights = Image("lights", "light.png")
image_shield = Image("shield", "shield.png")
image_explosion = Image("explosion", "fx/explosion.png")
image_electric = Image("electric", "fx/electric.png")
image_electrohit = Image("electrohit", "fx/electrohit.png")
image_electromine = Image("electromine", "fx/electromine.png")
image_mine1 = Image("mine1", "fx/mine1.png")
image_mine2 = Image("mine2", "fx/mine2.png")
image_smoke1 = Image("smoke1", "fx/smoke1.png")
image_death = Image("death", "fx/death.png")
image_lazer = Image("lazer", "fx/lazer.png")
image_lazerload = Image("lazerload", "fx/lazerload.png")
image_sparks = Image("sparks", "fx/sparks.png")
image_swordhit = Image("swordhit", "fx/swordhit.png")
image_bloodfx = Image("bloodfx", "fx/bloodfx.png")
image_playerspawn = Image("playerspawn", "fx/playerspawn.png")
image_monsterspawn = Image("monsterspawn", "fx/monsterspawn.png")
image_heal = Image("heal", "fx/heal.png")

image_texasmask = Image("texasmask", "texasmask.png")

container.images.Add(image_null)
container.images.Add(image_game)
container.images.Add(image_flag)
container.images.Add(image_weapons)
container.images.Add(image_sword)
container.images.Add(image_items)
container.images.Add(image_itemnumbers)
container.images.Add(image_hand)
container.images.Add(image_bodies)
container.images.Add(image_pickups)
container.images.Add(image_deathtypes)
container.images.Add(image_buildings)
container.images.Add(image_particles)
container.images.Add(image_gore)
container.images.Add(image_splatter)
container.images.Add(Image("cursor", "gui_cursor.png"))
container.images.Add(Image("banner", "gui_logo.png"))
container.images.Add(image_emoticons)
container.images.Add(image_browseicons)
container.images.Add(Image("console_bg", "console.png"))
container.images.Add(Image("console_bar", "console_bar.png"))
container.images.Add(image_demobuttons)
container.images.Add(image_fileicons)
container.images.Add(image_guibuttons)
container.images.Add(image_guiicons)
container.images.Add(image_lights)
container.images.Add(image_shield)
container.images.Add(image_explosion)
container.images.Add(image_electric)
container.images.Add(image_electrohit)
container.images.Add(image_electromine)
container.images.Add(image_mine1)
container.images.Add(image_mine2)
container.images.Add(image_smoke1)
container.images.Add(image_death)
container.images.Add(image_lazer)
container.images.Add(image_lazerload)
container.images.Add(image_sparks)
container.images.Add(image_swordhit)
container.images.Add(image_bloodfx)
container.images.Add(image_playerspawn)
container.images.Add(image_monsterspawn)
container.images.Add(image_heal)
container.images.Add(image_texasmask)

container.pickups.Add(Pickup("health"))
container.pickups.Add(Pickup("armor"))
container.pickups.Add(Pickup("weapon"))

set_particles = SpriteSet("particles", image_particles, 8, 8)
set_gore = SpriteSet("gore", image_gore, 8, 2)
set_splatter = SpriteSet("splatter", image_splatter, 4, 1)
set_game = SpriteSet("game", image_game, 32, 16)
set_flag = SpriteSet("flag", image_flag, 8, 2)
set_weapons = SpriteSet("weapons", image_weapons, 16, 16)
set_sword = SpriteSet("sword", image_sword, 4, 1)
set_hand = SpriteSet("hand", image_hand, 1, 1)
set_bodies = SpriteSet("bodies", image_bodies, 4, 1)
set_pickups = SpriteSet("pickups", image_pickups, 8, 2)
set_deathtypes = SpriteSet("deathtypes", image_deathtypes, 8, 4)
set_items = SpriteSet("items", image_items, 8, 1)
set_itemnumbers = SpriteSet("itemnumbers", image_itemnumbers, 8, 2)
set_buildings = SpriteSet("buildings", image_buildings, 8, 4)
set_tee = SpriteSet("tee", image_null, 8, 4)
set_browseicons = SpriteSet("browseicons", image_browseicons, 4, 1)
set_emoticons = SpriteSet("emoticons", image_emoticons, 4, 4)
set_demobuttons = SpriteSet("demobuttons", image_demobuttons, 5, 1)
set_fileicons = SpriteSet("fileicons", image_fileicons, 8, 1)
set_guibuttons = SpriteSet("guibuttons", image_guibuttons, 12, 4)
set_guiicons = SpriteSet("guiicons", image_guiicons, 8, 2)
set_shield = SpriteSet("shield", image_shield, 4, 2)
set_explosion = SpriteSet("explosion", image_explosion, 8, 2)
set_electric = SpriteSet("electric", image_electric, 8, 1)
set_electrohit = SpriteSet("electrohit", image_electrohit, 4, 1)
set_electromine = SpriteSet("electromine", image_electromine, 8, 2)
set_mine1 = SpriteSet("mine1", image_mine1, 8, 1)
set_mine2 = SpriteSet("mine2", image_mine2, 4, 1)
set_smoke1 = SpriteSet("smoke1", image_smoke1, 4, 4)
set_death = SpriteSet("death", image_death, 8, 1)
set_lazer = SpriteSet("lazer", image_lazer, 16, 1)
set_lazerload = SpriteSet("lazerload", image_lazerload, 8, 1)
set_sparks = SpriteSet("sparks", image_sparks, 4, 2)
set_swordhit = SpriteSet("swordhit", image_swordhit, 3, 1)
set_bloodfx = SpriteSet("bloodfx", image_bloodfx, 8, 1)
set_playerspawn = SpriteSet("playerspawn", image_playerspawn, 16, 1)
set_monsterspawn = SpriteSet("monsterspawn", image_monsterspawn, 16, 1)
set_heal = SpriteSet("heal", image_heal, 8, 2)

container.spritesets.Add(set_particles)
container.spritesets.Add(set_gore)
container.spritesets.Add(set_splatter)
container.spritesets.Add(set_game)
container.spritesets.Add(set_flag)
container.spritesets.Add(set_weapons)
container.spritesets.Add(set_items)
container.spritesets.Add(set_itemnumbers)
container.spritesets.Add(set_sword)
container.spritesets.Add(set_hand)
container.spritesets.Add(set_bodies)
container.spritesets.Add(set_pickups)
container.spritesets.Add(set_deathtypes)
container.spritesets.Add(set_buildings)
container.spritesets.Add(set_tee)
container.spritesets.Add(set_browseicons)
container.spritesets.Add(set_emoticons)
container.spritesets.Add(set_demobuttons)
container.spritesets.Add(set_fileicons)
container.spritesets.Add(set_guibuttons)
container.spritesets.Add(set_guiicons)
container.spritesets.Add(set_shield)
container.spritesets.Add(set_explosion)
container.spritesets.Add(set_electric)
container.spritesets.Add(set_electrohit)
container.spritesets.Add(set_electromine)
container.spritesets.Add(set_mine1)
container.spritesets.Add(set_mine2)
container.spritesets.Add(set_smoke1)
container.spritesets.Add(set_death)
container.spritesets.Add(set_lazer)
container.spritesets.Add(set_lazerload)
container.spritesets.Add(set_sparks)
container.spritesets.Add(set_swordhit)
container.spritesets.Add(set_bloodfx)
container.spritesets.Add(set_playerspawn)
container.spritesets.Add(set_monsterspawn)
container.spritesets.Add(set_heal)


container.sprites.Add(Sprite("shield1", set_shield, 0,0,1,1))
container.sprites.Add(Sprite("shield2", set_shield, 1,0,1,1))
container.sprites.Add(Sprite("shield3", set_shield, 2,0,1,1))
container.sprites.Add(Sprite("shield4", set_shield, 3,0,1,1))
container.sprites.Add(Sprite("shield5", set_shield, 0,1,1,1))
container.sprites.Add(Sprite("shield6", set_shield, 1,1,1,1))
container.sprites.Add(Sprite("shield7", set_shield, 2,1,1,1))
container.sprites.Add(Sprite("shield8", set_shield, 3,1,1,1))



# sprite sheets

# 3 frames
for i in range(1, 4):
	container.sprites.Add(Sprite("spark1_"+str(i), set_sparks, i-1, 0, 1, 1))
for i in range(1, 4):
	container.sprites.Add(Sprite("spark2_"+str(i), set_sparks, i-1, 1, 1, 1))
	
for i in range(1, 4):
	container.sprites.Add(Sprite("swordhit"+str(i), set_swordhit, i-1, 1, 1, 1))

# 4 frames
for i in range(1, 5):
	container.sprites.Add(Sprite("body"+str(i), set_bodies, i-1, 0, 1, 1))
	
for i in range(1, 5):
	container.sprites.Add(Sprite("mine2_"+str(i), set_mine2, i-1, 0, 1, 1))

for i in range(1, 5):
	container.sprites.Add(Sprite("electrohit"+str(i), set_electrohit, i-1, 0, 1, 1))
	
for i in range(1, 5):
	container.sprites.Add(Sprite("sword"+str(i), set_sword, i-1, 0, 1, 1))
	
# 8 + 4

for i in range(1, 9):
	container.sprites.Add(Sprite("heal"+str(i), set_heal, i-1,0,1,1))
for i in range(1, 5):
	container.sprites.Add(Sprite("heal"+str(8+i), set_heal, i-1,1,1,1))
	
for i in range(1, 9):
	container.sprites.Add(Sprite("electromine"+str(i), set_electromine, i-1,0,1,1))
for i in range(1, 5):
	container.sprites.Add(Sprite("electromine"+str(8+i), set_electromine, i-1,1,1,1))
	
# 8 frames
for i in range(1, 9):
	container.sprites.Add(Sprite("deathtype"+str(i), set_deathtypes, i-1,0,1,1))
for i in range(1, 9):
	container.sprites.Add(Sprite("deathtype"+str(8+i), set_deathtypes, i-1,1,1,1))
for i in range(1, 9):
	container.sprites.Add(Sprite("deathtype"+str(16+i), set_deathtypes, i-1,2,1,1))
for i in range(1, 9):
	container.sprites.Add(Sprite("deathtype"+str(24+i), set_deathtypes, i-1,3,1,1))
	
for i in range(1, 9):
	container.sprites.Add(Sprite("explosion1_"+str(i), set_explosion, i-1,0,1,1))
for i in range(1, 9):
	container.sprites.Add(Sprite("explosion2_"+str(i), set_explosion, i-1,1,1,1))
for i in range(1, 9):
	container.sprites.Add(Sprite("mine1_"+str(i), set_mine1, i-1, 0, 1, 1))
for i in range(1, 9):
	container.sprites.Add(Sprite("electric"+str(i), set_electric, i-1, 0, 1, 1))
for i in range(1, 9):
	container.sprites.Add(Sprite("lazerload"+str(i), set_lazerload, i-1, 0, 1, 1))
for i in range(1, 9):
	container.sprites.Add(Sprite("bloodfx1_"+str(i), set_bloodfx, i-1, 0, 1, 1))
	
for i in range(1, 9):
	container.sprites.Add(Sprite("death"+str(i), set_death, i-1, 0, 1, 1))
	
for i in range(1, 9):
	container.sprites.Add(Sprite("item"+str(i), set_items, i-1,0,1,1))

for i in range(1, 9):
	container.sprites.Add(Sprite("itemnumber_"+str(i-1), set_itemnumbers, i-1, 0, 1, 1))

container.sprites.Add(Sprite("itemnumber_8", set_itemnumbers, 0, 1, 1, 1))
container.sprites.Add(Sprite("itemnumber_9", set_itemnumbers, 1, 1, 1, 1))
	
# 11
for i in range(1, 12):
	container.sprites.Add(Sprite("playerspawn"+str(i), set_playerspawn, i-1, 0, 1, 1))
for i in range(1, 12):
	container.sprites.Add(Sprite("monsterspawn"+str(i), set_monsterspawn, i-1, 0, 1, 1))
	
# 16
for i in range(1, 17):
	container.sprites.Add(Sprite("lazer"+str(i), set_lazer, i-1, 0, 1, 1))
	
# 4 x 4
for y in range(1, 4):
	for x in range(1, 5):
		container.sprites.Add(Sprite("smoke1_"+str((y-1)*4 + x-1), set_smoke1, x-1, y-1, 1, 1))

	
container.sprites.Add(Sprite("hand", set_hand, 0,0,1,1))

container.sprites.Add(Sprite("part_slice", set_particles, 0,0,1,1))
container.sprites.Add(Sprite("part_ball", set_particles, 1,0,1,1))
container.sprites.Add(Sprite("part_splat01", set_particles, 2,0,1,1))
container.sprites.Add(Sprite("part_splat02", set_particles, 3,0,1,1))
container.sprites.Add(Sprite("part_splat03", set_particles, 4,0,1,1))
container.sprites.Add(Sprite("part_bullettrace", set_particles, 4,3,1,1))

container.sprites.Add(Sprite("part_particle1", set_particles, 6,0,1,1))
container.sprites.Add(Sprite("part_particle2", set_particles, 7,0,1,1))
container.sprites.Add(Sprite("part_particle3", set_particles, 6,1,1,1))
container.sprites.Add(Sprite("part_particle4", set_particles, 7,1,1,1))

container.sprites.Add(Sprite("part_smoke", set_particles, 0,1,1,1))
container.sprites.Add(Sprite("part_shell", set_particles, 0,2,2,2))
container.sprites.Add(Sprite("part_expl01", set_particles, 0,4,4,4))
container.sprites.Add(Sprite("part_airjump", set_particles, 2,2,2,2))
container.sprites.Add(Sprite("part_hit01", set_particles, 4,1,2,2))


container.sprites.Add(Sprite("blood01", set_gore, 0,0,2,1))
container.sprites.Add(Sprite("blood02", set_gore, 2,0,2,1))
container.sprites.Add(Sprite("blood03", set_gore, 0,1,2,1))
container.sprites.Add(Sprite("blood04", set_gore, 2,1,2,1))
container.sprites.Add(Sprite("blood05", set_gore, 4,1,2,1))
container.sprites.Add(Sprite("blood06", set_gore, 6,1,2,1))
container.sprites.Add(Sprite("bone01", set_gore, 4,0,2,1))
container.sprites.Add(Sprite("bone02", set_gore, 6,0,2,1))


container.sprites.Add(Sprite("splatter01", set_splatter, 0,0,1,1))
container.sprites.Add(Sprite("splatter02", set_splatter, 1,0,1,1))
container.sprites.Add(Sprite("splatter03", set_splatter, 2,0,1,1))
container.sprites.Add(Sprite("splatter04", set_splatter, 3,0,1,1))


container.sprites.Add(Sprite("health_full", set_game, 21,0,2,2))
container.sprites.Add(Sprite("health_empty", set_game, 23,0,2,2))
container.sprites.Add(Sprite("armor_full", set_game, 21,2,2,2))
container.sprites.Add(Sprite("armor_empty", set_game, 23,2,2,2))

container.sprites.Add(Sprite("star1", set_game, 15,0,2,2))
container.sprites.Add(Sprite("star2", set_game, 17,0,2,2))
container.sprites.Add(Sprite("star3", set_game, 19,0,2,2))

container.sprites.Add(Sprite("part1", set_game, 6,0,1,1))
container.sprites.Add(Sprite("part2", set_game, 6,1,1,1))
container.sprites.Add(Sprite("part3", set_game, 7,0,1,1))
container.sprites.Add(Sprite("part4", set_game, 7,1,1,1))
container.sprites.Add(Sprite("part5", set_game, 8,0,1,1))
container.sprites.Add(Sprite("part6", set_game, 8,1,1,1))
container.sprites.Add(Sprite("part7", set_game, 9,0,2,2))
container.sprites.Add(Sprite("part8", set_game, 11,0,2,2))
container.sprites.Add(Sprite("part9", set_game, 13,0,2,2))


container.sprites.Add(Sprite("weapon_pickup", set_weapons, 8,12,4,4))

container.sprites.Add(Sprite("weapon_tool_body", set_weapons, 0,0,4,2))
container.sprites.Add(Sprite("weapon_tool_pickup", set_pickups, 0,0,0,0))
container.sprites.Add(Sprite("weapon_tool_cursor", set_game, 0,0,2,2))
container.sprites.Add(Sprite("weapon_tool_proj", set_game, 0,0,0,0))

container.sprites.Add(Sprite("weapon_hammer_body", set_weapons, 8,0,8,2))
container.sprites.Add(Sprite("weapon_hammer_pickup", set_pickups, 0,0,0,0))
container.sprites.Add(Sprite("weapon_hammer_cursor", set_game, 0,0,2,2))
container.sprites.Add(Sprite("weapon_hammer_proj", set_game, 0,0,0,0))

container.sprites.Add(Sprite("weapon_chainsaw_body", set_weapons, 4,2,6,2))
container.sprites.Add(Sprite("weapon_chainsaw_pickup", set_pickups, 6,0,1,1))
container.sprites.Add(Sprite("weapon_chainsaw_cursor", set_game, 0,0,2,2))
container.sprites.Add(Sprite("weapon_chainsaw_proj", set_game, 0,0,0,0))

container.sprites.Add(Sprite("weapon_gun_body", set_weapons, 4,0,4,2))
container.sprites.Add(Sprite("weapon_gun_pickup", set_pickups, 0,0,1,1))
container.sprites.Add(Sprite("weapon_gun_cursor", set_game, 0,4,2,2))
container.sprites.Add(Sprite("weapon_gun_proj", set_game, 6,4,2,2))
container.sprites.Add(Sprite("weapon_gun_muzzle1", set_game, 8,4,3,2))
container.sprites.Add(Sprite("weapon_gun_muzzle2", set_game, 12,4,3,2))
container.sprites.Add(Sprite("weapon_gun_muzzle3", set_game, 16,4,3,2))

container.sprites.Add(Sprite("weapon_shotgun_body", set_weapons, 0,5,8,2))
container.sprites.Add(Sprite("weapon_shotgun_pickup", set_pickups, 1,0,1,1))
container.sprites.Add(Sprite("weapon_shotgun_cursor", set_game, 0,6,2,2))
container.sprites.Add(Sprite("weapon_shotgun_proj", set_game, 10,6,2,2))
container.sprites.Add(Sprite("weapon_shotgun_muzzle1", set_game, 12,6,3,2))
container.sprites.Add(Sprite("weapon_shotgun_muzzle2", set_game, 16,6,3,2))
container.sprites.Add(Sprite("weapon_shotgun_muzzle3", set_game, 20,6,3,2))

container.sprites.Add(Sprite("weapon_grenade_body", set_weapons, 0,7,7,2))
container.sprites.Add(Sprite("weapon_grenade_pickup", set_pickups, 4,0,1,1))
container.sprites.Add(Sprite("weapon_grenade_cursor", set_game, 0,8,2,2))
container.sprites.Add(Sprite("weapon_grenade_proj", set_game, 10,8,2,2))

container.sprites.Add(Sprite("weapon_rifle_body", set_weapons, 0,9,7,3))
container.sprites.Add(Sprite("weapon_rifle_pickup", set_pickups, 2,0,1,1))
container.sprites.Add(Sprite("weapon_rifle_cursor", set_game, 0,12,2,2))
container.sprites.Add(Sprite("weapon_rifle_proj", set_game, 10,12,2,2))
container.sprites.Add(Sprite("weapon_rifle_muzzle1", set_game, 8,4,3,2))
container.sprites.Add(Sprite("weapon_rifle_muzzle2", set_game, 12,4,3,2))
container.sprites.Add(Sprite("weapon_rifle_muzzle3", set_game, 16,4,3,2))

container.sprites.Add(Sprite("weapon_electric_body", set_weapons, 8,8,7,3))
container.sprites.Add(Sprite("weapon_electric_pickup", set_pickups, 0,1,1,1))
container.sprites.Add(Sprite("weapon_electric_cursor", set_game, 0,12,2,2))
container.sprites.Add(Sprite("weapon_electric_proj", set_game, 10,10,2,2))
container.sprites.Add(Sprite("weapon_electric_muzzle1", set_game, 8,4,3,2))
container.sprites.Add(Sprite("weapon_electric_muzzle2", set_game, 12,4,3,2))
container.sprites.Add(Sprite("weapon_electric_muzzle3", set_game, 16,4,3,2))

container.sprites.Add(Sprite("weapon_laser_body", set_weapons, 0,12,7,3))
container.sprites.Add(Sprite("weapon_laser_pickup", set_pickups, 3,0,1,1))
container.sprites.Add(Sprite("weapon_laser_cursor", set_game, 0,12,2,2))
container.sprites.Add(Sprite("weapon_laser_proj", set_game, 10,12,2,2))

container.sprites.Add(Sprite("weapon_flamer_body", set_weapons, 8,5,8,3))
container.sprites.Add(Sprite("weapon_flamer_pickup", set_pickups, 5,0,1,1))
container.sprites.Add(Sprite("weapon_flamer_cursor", set_game, 0,6,2,2))
container.sprites.Add(Sprite("weapon_flamer_proj", set_game, 6,0,1,1))

container.sprites.Add(Sprite("hook_chain", set_game, 2,0,1,1))
container.sprites.Add(Sprite("hook_head", set_game, 3,0,2,1))

container.sprites.Add(Sprite("pickup_mine", set_weapons, 14,12,2,2))
container.sprites.Add(Sprite("pickup_health", set_weapons, 12,14,2,2))
container.sprites.Add(Sprite("pickup_armor", set_weapons, 14,14,2,2))
container.sprites.Add(Sprite("pickup_weapon", set_game, 3,0,6,2))

# buildings.png
container.sprites.Add(Sprite("mine1", set_buildings, 0,0,2,1))
container.sprites.Add(Sprite("mine2", set_buildings, 0,1,2,1))
container.sprites.Add(Sprite("sawblade", set_buildings, 4,0,2,2))
container.sprites.Add(Sprite("lazer", set_buildings, 2,0,2,1))
container.sprites.Add(Sprite("powerupper", set_buildings, 2,1,2,1))

for i in range(1, 9):
	container.sprites.Add(Sprite("barrel"+str(i), set_buildings, i-1,2,1,2))


container.sprites.Add(Sprite("flag_blue", set_game, 12,8,4,8))
container.sprites.Add(Sprite("flag_red", set_game, 16,8,4,8))

container.sprites.Add(Sprite("fireball1", set_game, 2, 7, 5, 2))
container.sprites.Add(Sprite("fireball2", set_game, 2, 9, 5, 2))
container.sprites.Add(Sprite("fireball3", set_game, 2, 11, 5, 2))



container.sprites.Add(Sprite("flag_blue1", set_flag, 0,0,1,1))
container.sprites.Add(Sprite("flag_blue2", set_flag, 1,0,1,1))
container.sprites.Add(Sprite("flag_blue3", set_flag, 2,0,1,1))
container.sprites.Add(Sprite("flag_blue4", set_flag, 3,0,1,1))
container.sprites.Add(Sprite("flag_blue5", set_flag, 4,0,1,1))
container.sprites.Add(Sprite("flag_blue6", set_flag, 5,0,1,1))

container.sprites.Add(Sprite("flag_red1", set_flag, 0,1,1,1))
container.sprites.Add(Sprite("flag_red2", set_flag, 1,1,1,1))
container.sprites.Add(Sprite("flag_red3", set_flag, 2,1,1,1))
container.sprites.Add(Sprite("flag_red4", set_flag, 3,1,1,1))
container.sprites.Add(Sprite("flag_red5", set_flag, 4,1,1,1))
container.sprites.Add(Sprite("flag_red6", set_flag, 5,1,1,1))



container.sprites.Add(Sprite("tee_body", set_tee, 0,0,3,3))
container.sprites.Add(Sprite("tee_body_outline", set_tee, 3,0,3,3))
container.sprites.Add(Sprite("tee_foot", set_tee, 6,1,2,1))
container.sprites.Add(Sprite("tee_foot_outline", set_tee, 6,2,2,1))
container.sprites.Add(Sprite("tee_hand", set_tee, 6,0,1,1))
container.sprites.Add(Sprite("tee_hand_outline", set_tee, 7,0,1,1))
container.sprites.Add(Sprite("tee_eye_normal", set_tee, 2,3,1,1))
container.sprites.Add(Sprite("tee_eye_angry", set_tee, 3,3,1,1))
container.sprites.Add(Sprite("tee_eye_pain", set_tee, 4,3,1,1))
container.sprites.Add(Sprite("tee_eye_happy", set_tee, 5,3,1,1))
container.sprites.Add(Sprite("tee_eye_dead", set_tee, 6,3,1,1))
container.sprites.Add(Sprite("tee_eye_surprise", set_tee, 7,3,1,1))

container.sprites.Add(Sprite("oop", set_emoticons, 0, 0, 1, 1))
container.sprites.Add(Sprite("exclamation", set_emoticons, 1, 0, 1, 1))
container.sprites.Add(Sprite("hearts", set_emoticons, 2, 0, 1, 1))
container.sprites.Add(Sprite("drop", set_emoticons, 3, 0, 1, 1))
container.sprites.Add(Sprite("dotdot", set_emoticons, 0, 1, 1, 1))
container.sprites.Add(Sprite("music", set_emoticons, 1, 1, 1, 1))
container.sprites.Add(Sprite("sorry", set_emoticons, 2, 1, 1, 1))
container.sprites.Add(Sprite("ghost", set_emoticons, 3, 1, 1, 1))
container.sprites.Add(Sprite("sushi", set_emoticons, 0, 2, 1, 1))
container.sprites.Add(Sprite("splattee", set_emoticons, 1, 2, 1, 1))
container.sprites.Add(Sprite("deviltee", set_emoticons, 2, 2, 1, 1))
container.sprites.Add(Sprite("zomg", set_emoticons, 3, 2, 1, 1))
container.sprites.Add(Sprite("zzz", set_emoticons, 0, 3, 1, 1))
container.sprites.Add(Sprite("wtf", set_emoticons, 1, 3, 1, 1))
container.sprites.Add(Sprite("eyes", set_emoticons, 2, 3, 1, 1))
container.sprites.Add(Sprite("question", set_emoticons, 3, 3, 1, 1))

container.sprites.Add(Sprite("browse_lock", set_browseicons, 0,0,1,1))
container.sprites.Add(Sprite("browse_heart", set_browseicons, 1,0,1,1))
container.sprites.Add(Sprite("browse_unpure", set_browseicons, 3,0,1,1))

container.sprites.Add(Sprite("demobutton_play", set_demobuttons, 0,0,1,1))
container.sprites.Add(Sprite("demobutton_pause", set_demobuttons, 1,0,1,1))
container.sprites.Add(Sprite("demobutton_stop", set_demobuttons, 2,0,1,1))
container.sprites.Add(Sprite("demobutton_slower", set_demobuttons, 3,0,1,1))
container.sprites.Add(Sprite("demobutton_faster", set_demobuttons, 4,0,1,1))

container.sprites.Add(Sprite("file_demo1", set_fileicons, 0,0,1,1))
container.sprites.Add(Sprite("file_demo2", set_fileicons, 1,0,1,1))
container.sprites.Add(Sprite("file_folder", set_fileicons, 2,0,1,1))
container.sprites.Add(Sprite("file_map1", set_fileicons, 5,0,1,1))
container.sprites.Add(Sprite("file_map2", set_fileicons, 6,0,1,1))

container.sprites.Add(Sprite("guibutton_off", set_guibuttons, 0,0,4,4))
container.sprites.Add(Sprite("guibutton_on", set_guibuttons, 4,0,4,4))
container.sprites.Add(Sprite("guibutton_hover", set_guibuttons, 8,0,4,4))

container.sprites.Add(Sprite("guiicon_mute", set_guiicons, 0,0,4,2))
container.sprites.Add(Sprite("guiicon_friend", set_guiicons, 4,0,4,2))



weapon = WeaponSpec(container, "tool")
weapon.firedelay.Set(125)
weapon.damage.Set(1)
weapon.visual_size.Set(96)
weapon.offsetx.Set(4)
weapon.offsety.Set(-20)
container.weapons.tool.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "hammer")
weapon.firedelay.Set(125)
weapon.damage.Set(3)
weapon.visual_size.Set(96)
weapon.offsetx.Set(4)
weapon.offsety.Set(-20)
container.weapons.hammer.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "chainsaw")
weapon.firedelay.Set(125)
weapon.damage.Set(3)
weapon.visual_size.Set(96)
weapon.offsetx.Set(25)
weapon.offsety.Set(-2)
container.weapons.chainsaw.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "gun")
weapon.firedelay.Set(125)
weapon.ammoregentime.Set(500)
weapon.visual_size.Set(64)
weapon.offsetx.Set(32)
weapon.offsety.Set(4)
weapon.muzzleoffsetx.Set(50)
weapon.muzzleoffsety.Set(6)
container.weapons.gun.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "shotgun")
weapon.firedelay.Set(500)
weapon.visual_size.Set(96)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
weapon.muzzleoffsetx.Set(70)
weapon.muzzleoffsety.Set(6)
container.weapons.shotgun.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "grenade")
weapon.firedelay.Set(500) # TODO: fix this
weapon.visual_size.Set(96)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
container.weapons.grenade.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "rifle")
weapon.firedelay.Set(180)
weapon.visual_size.Set(92)
weapon.damage.Set(5)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
weapon.muzzleoffsetx.Set(75)
weapon.muzzleoffsety.Set(6)
container.weapons.rifle.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "electric")
weapon.firedelay.Set(180)
weapon.visual_size.Set(92)
weapon.damage.Set(5)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
weapon.muzzleoffsetx.Set(75)
weapon.muzzleoffsety.Set(6)
container.weapons.electric.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "laser")
weapon.firedelay.Set(800)
weapon.visual_size.Set(92)
weapon.damage.Set(5)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
container.weapons.laser.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "flamer")
weapon.firedelay.Set(180)
weapon.visual_size.Set(92+12)
weapon.damage.Set(5)
weapon.offsetx.Set(30)
weapon.offsety.Set(-2)
container.weapons.flamer.base.Set(weapon)
container.weapons.id.Add(weapon)
