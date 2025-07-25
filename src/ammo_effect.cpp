#include "ammo_effect.h"

#include "debug.h"
#include "flexbuffer_json.h"
#include "generic_factory.h"

generic_factory<ammo_effect> &get_all_ammo_effects()
{
    static generic_factory<ammo_effect> all_ammo_effects( "ammo effects" );
    return all_ammo_effects;
}

/** @relates int_id */
template<>
bool int_id<ammo_effect>::is_valid() const
{
    return get_all_ammo_effects().is_valid( *this );
}

/** @relates int_id */
template<>
const ammo_effect &int_id<ammo_effect>::obj() const
{
    return get_all_ammo_effects().obj( *this );
}

/** @relates int_id */
template<>
const ammo_effect_str_id &int_id<ammo_effect>::id() const
{
    return get_all_ammo_effects().convert( *this );
}

/** @relates string_id */
template<>
bool ammo_effect_str_id::is_valid() const
{
    return get_all_ammo_effects().is_valid( *this );
}

/** @relates string_id */
template<>
const ammo_effect &ammo_effect_str_id::obj() const
{
    return get_all_ammo_effects().obj( *this );
}

/** @relates string_id */
template<>
int_id<ammo_effect> ammo_effect_str_id::id() const
{
    return get_all_ammo_effects().convert( *this, AE_NULL );
}

/** @relates int_id */
template<>
int_id<ammo_effect>::int_id( const ammo_effect_str_id &id ) : _id( id.id() )
{
}

void on_hit_effect::deserialize( const JsonObject &jo )
{
    optional( jo, false, "need_touch_skin", need_touch_skin, false );
    mandatory( jo, false, "duration", duration );
    mandatory( jo, false, "effect", effect );
    mandatory( jo, false, "intensity", intensity );
}

void ammo_effect::load( const JsonObject &jo, std::string_view )
{
    optional( jo, was_loaded, "trigger_chance", trigger_chance, 1 );

    if( jo.has_member( "aoe" ) ) {
        JsonObject joa = jo.get_object( "aoe" );
        optional( joa, was_loaded, "field_type", aoe_field_type_name, "fd_null" );
        optional( joa, was_loaded, "intensity_min", aoe_intensity_min, 0 );
        optional( joa, was_loaded, "intensity_max", aoe_intensity_max, 0 );
        optional( joa, was_loaded, "radius", aoe_radius, 1 );
        optional( joa, was_loaded, "radius_z", aoe_radius_z, 0 );
        optional( joa, was_loaded, "chance", aoe_chance, 100 );
        optional( joa, was_loaded, "size", aoe_size, 0 );
        optional( joa, was_loaded, "check_passable", aoe_check_passable, false );
        optional( joa, was_loaded, "check_sees", aoe_check_sees, false );
        optional( joa, was_loaded, "check_sees_radius", aoe_check_sees_radius, 0 );
    }
    if( jo.has_member( "trail" ) ) {
        JsonObject joa = jo.get_object( "trail" );
        optional( joa, was_loaded, "field_type", trail_field_type_name, "fd_null" );
        optional( joa, was_loaded, "intensity_min", trail_intensity_min, 0 );
        optional( joa, was_loaded, "intensity_max", trail_intensity_max, 0 );
        optional( joa, was_loaded, "chance", trail_chance, 100 );
    }
    optional( jo, was_loaded, "on_hit_effects", on_hit_effects );
    optional( jo, was_loaded, "explosion", aoe_explosion_data );
    optional( jo, was_loaded, "do_flashbang", do_flashbang, false );
    optional( jo, was_loaded, "do_emp_blast", do_emp_blast, false );
    optional( jo, was_loaded, "foamcrete_build", foamcrete_build, false );
    optional( jo, was_loaded, "eoc", eoc );
    optional( jo, was_loaded, "spell_data", spell_data );
    optional( jo, was_loaded, "always_cast_spell", always_cast_spell, false );


}

void ammo_effect::finalize()
{
    for( const ammo_effect &ae : ammo_effects::get_all() ) {
        const_cast<ammo_effect &>( ae ).aoe_field_type = field_type_id( ae.aoe_field_type_name );
        const_cast<ammo_effect &>( ae ).trail_field_type = field_type_id( ae.trail_field_type_name );
    }

}

void ammo_effect::check() const
{
    if( !aoe_field_type.is_valid() ) {
        debugmsg( "No such field type %s", aoe_field_type_name );
    }
    if( aoe_check_sees_radius < 0 ) {
        debugmsg( "Value of aoe_check_sees_radius cannot be negative" );
    }
    if( aoe_size < 0 ) {
        debugmsg( "Value of aoe_size cannot be negative" );
    }
    if( aoe_chance > 100 || aoe_chance <= 0 ) {
        debugmsg( "Field chance of %s out of range (%d of min 1 max 100)", id.c_str(), aoe_chance );
    }
    if( aoe_radius_z < 0 || aoe_radius < 0 ) {
        debugmsg( "Radius values cannot be negative" );
    }
    if( aoe_intensity_min < 0 ) {
        debugmsg( "Field intensity cannot be negative" );
    }
    if( aoe_intensity_max < aoe_intensity_min ) {
        debugmsg( "Maximum intensity must be greater than or equal to minimum intensity" );
    }
    if( !trail_field_type.is_valid() ) {
        debugmsg( "No such field type %s", trail_field_type_name );
    }
    if( trail_chance > 100 || trail_chance <= 0 ) {
        debugmsg( "Field chance divisor cannot be negative" );
        debugmsg( "Field chance of %s out of range (%d of min 1 max 100)", id.c_str(), trail_chance );
    }
    if( trail_intensity_min < 0 ) {
        debugmsg( "Field intensity cannot be negative" );
    }
    if( trail_intensity_max < trail_intensity_min ) {
        debugmsg( "Maximum intensity must be greater than or equal to minimum intensity" );
    }
}

size_t ammo_effect::count()
{
    return get_all_ammo_effects().size();
}

void ammo_effects::load( const JsonObject &jo, const std::string &src )
{
    get_all_ammo_effects().load( jo, src );
}

void ammo_effects::finalize_all()
{
    get_all_ammo_effects().finalize();
    for( const ammo_effect &ae : get_all_ammo_effects().get_all() ) {
        const_cast<ammo_effect &>( ae ).finalize();
    }
}

void ammo_effects::check_consistency()
{
    get_all_ammo_effects().check();
}

void ammo_effects::reset()
{
    get_all_ammo_effects().reset();
}

const std::vector<ammo_effect> &ammo_effects::get_all()
{
    return get_all_ammo_effects().get_all();
}

ammo_effect_id AE_NULL;
