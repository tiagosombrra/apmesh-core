// Fixed-profile observation adapter. Expected results belong to the Python oracle.
#include "apmesh/core/geometry.hpp"
#include <array>
#include <cmath>
#include <concepts>
#include <expected>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {
using namespace apmesh::core;
template<class F, class P> concept PW = requires(F f, P p) { f.point_to_world(p); };
template<class F, class P> concept PL = requires(F f, P p) { f.point_to_local(p); };
template<class F, class V> concept VW = requires(F f, V v) { f.vector_to_world(v); };
template<class F, class V> concept VL = requires(F f, V v) { f.vector_to_local(v); };
static_assert(PW<CartesianFrame2, Point2> && PL<CartesianFrame2, Point2>);
static_assert(VW<CartesianFrame2, Vector2> && VL<CartesianFrame2, Vector2>);
static_assert(PW<CartesianFrame3, Point3> && PL<CartesianFrame3, Point3>);
static_assert(VW<CartesianFrame3, Vector3> && VL<CartesianFrame3, Vector3>);
static_assert(!PW<CartesianFrame2, Point3> && !PL<CartesianFrame2, Point3>);
static_assert(!VW<CartesianFrame2, Vector3> && !VL<CartesianFrame2, Vector3>);
static_assert(!PW<CartesianFrame3, Point2> && !PL<CartesianFrame3, Point2>);
static_assert(!VW<CartesianFrame3, Vector2> && !VL<CartesianFrame3, Vector2>);
static_assert(!PW<CartesianFrame2, Vector2> && !PL<CartesianFrame2, Vector2>);
static_assert(!VW<CartesianFrame2, Point2> && !VL<CartesianFrame2, Point2>);
static_assert(!PW<CartesianFrame3, Vector3> && !PL<CartesianFrame3, Vector3>);
static_assert(!VW<CartesianFrame3, Point3> && !VL<CartesianFrame3, Point3>);
static_assert(std::is_same_v<decltype(std::declval<const CartesianFrame2&>().basis()), const Mat2&>);
static_assert(std::is_same_v<decltype(std::declval<const CartesianFrame3&>().origin()), const Point3&>);

struct Case {
    std::string id, operation, kind, direction, category;
    int dimension{}, exponent{};
    std::vector<double> origin, basis, operand, auxiliary;
};
struct Observation { std::string error; std::vector<double> value; };
std::string name(GeometryError e) {
    switch(e) {
    case GeometryError::non_finite_input: return "non_finite_input";
    case GeometryError::non_finite_result: return "non_finite_result";
    case GeometryError::invalid_frame: return "invalid_frame";
    case GeometryError::scale_out_of_range: return "scale_out_of_range";
    default: throw std::runtime_error("unexpected geometry error");
    }
}
template<class T> T checked(std::expected<T, GeometryError> v) {
    if (!v) throw v.error();
    return *v;
}
template<class T> std::vector<double> flat(const T& v) {
    if constexpr (requires { v.z(); }) return {v.x(),v.y(),v.z()};
    else return {v.x(),v.y()};
}
template<class T> T value(const std::vector<double>& x) {
    if constexpr (std::is_same_v<T,Point2> || std::is_same_v<T,Vector2>) return checked(T::make(x[0],x[1]));
    else return checked(T::make(x[0],x[1],x[2]));
}
template<int N> Observation observe(const Case& c) {
    using P=std::conditional_t<N==2,Point2,Point3>;
    using V=std::conditional_t<N==2,Vector2,Vector3>;
    using M=std::conditional_t<N==2,Mat2,Mat3>;
    using F=std::conditional_t<N==2,CartesianFrame2,CartesianFrame3>;
    std::array<double,N*N> entries{};
    for(std::size_t i=0;i<entries.size();++i) entries[i]=c.basis[i];
    const auto matrix=M::make(entries);
    if(!matrix) {
        if(matrix.error()!=LinearAlgebraError::non_finite_input) throw std::runtime_error("unexpected matrix error");
        return {"non_finite_input",{}};
    }
    try {
        const F frame=c.operation=="identity"?F::identity():checked(F::make(value<P>(c.origin),*matrix,c.exponent));
        const auto point=[&](const P& p,bool world){return checked(world?frame.point_to_world(p):frame.point_to_local(p));};
        const auto vector=[&](const V& v,bool world){return checked(world?frame.vector_to_world(v):frame.vector_to_local(v));};
        const bool world=c.direction=="world";
        std::vector<double> result;
        if(c.operation=="construct" || c.operation=="basis") {
            if(c.operation=="construct") result=flat(frame.origin());
            for(std::size_t r=0;r<N;++r) for(std::size_t col=0;col<N;++col) {
                const auto entry=frame.basis().at(r,col);
                if(!entry) throw std::runtime_error("basis accessor failure");
                result.push_back(*entry);
            }
            if(c.operation=="construct") result.push_back(static_cast<double>(frame.scale_exponent()));
        } else if(c.operation=="origin") result=flat(frame.origin());
        else if(c.operation=="exponent") result={static_cast<double>(frame.scale_exponent())};
        else if(c.operation=="type_separation") result={1.0};
        else if(c.operation=="map" || c.operation=="identity" || c.operation=="roundtrip") {
            if(c.kind=="point") {
                auto mapped=point(value<P>(c.operand),world);
                if(c.operation=="roundtrip") mapped=point(mapped,!world);
                result=flat(mapped);
            } else {
                auto mapped=vector(value<V>(c.operand),world);
                if(c.operation=="roundtrip") mapped=vector(mapped,!world);
                result=flat(mapped);
            }
        } else if(c.operation=="translation") {
            const F zero_frame=checked(F::make(value<P>(std::vector<double>(N,0.0)),*matrix,c.exponent));
            if(c.kind=="point") {
                const auto p=value<P>(c.operand);
                result=flat(point(p,world));
                const auto other=flat(checked(world?zero_frame.point_to_world(p):zero_frame.point_to_local(p)));
                result.insert(result.end(),other.begin(),other.end());
            } else {
                const auto v=value<V>(c.operand);
                result=flat(vector(v,world));
                const auto other=flat(checked(world?zero_frame.vector_to_world(v):zero_frame.vector_to_local(v)));
                result.insert(result.end(),other.begin(),other.end());
            }
        } else if(c.operation=="affine") {
            const auto p=value<P>(c.operand); const auto v=value<V>(c.auxiliary);
            result=flat(point(checked(p+v),true));
            const auto rhs=flat(checked(point(p,true)+vector(v,true)));
            result.insert(result.end(),rhs.begin(),rhs.end());
        } else if(c.operation=="difference") {
            const auto p=value<P>(c.operand); const auto q=value<P>(c.auxiliary);
            result=flat(vector(checked(p-q),true));
            const auto rhs=flat(checked(point(p,true)-point(q,true)));
            result.insert(result.end(),rhs.begin(),rhs.end());
        } else if(c.operation=="dot") {
            const auto v=value<V>(c.operand), w=value<V>(c.auxiliary);
            result={checked(dot(vector(v,true),vector(w,true))),std::ldexp(checked(dot(v,w)),2*c.exponent)};
        } else if(c.operation=="norm") {
            const auto v=value<V>(c.operand);
            result={checked(norm(vector(v,true))),std::ldexp(checked(norm(v)),c.exponent)};
        } else throw std::runtime_error("unknown operation");
        return {"",result};
    } catch(GeometryError e) {return {name(e),{}};}
}
void array(std::ostream& out,const std::vector<double>& v) {
    out<<'[';
    for(std::size_t i=0;i<v.size();++i) {if(i)out<<',';out<<'"'<<std::hexfloat<<v[i]<<'"';}
    out<<']';
}
double number(std::istream& in) {
    std::string token; if(!(in>>token))throw std::runtime_error("missing numeric input");
    std::size_t used{}; const double result=std::stod(token,&used);
    if(used!=token.size())throw std::runtime_error("invalid numeric input");
    return result;
}
void write_case(std::ostream& out,const Case& c,const Observation& obs) {
    out<<"{\"schema_version\":2,\"id\":\""<<c.id<<"\",\"dimension\":"<<c.dimension
       <<",\"operation\":\""<<c.operation<<"\",\"operand_kind\":\""<<c.kind
       <<"\",\"direction\":\""<<c.direction<<"\",\"claim_category\":\""<<c.category<<"\",\"inputs\":{\"origin\":";
    array(out,c.origin);out<<",\"basis\":";array(out,c.basis);out<<",\"exponent\":"<<c.exponent<<",\"operand\":";
    array(out,c.operand);out<<",\"auxiliary\":";array(out,c.auxiliary);
    out<<"},\"non_claims\":[\"orientation\",\"handedness\",\"topology\",\"general_transform\"],\"observed\":{\"kind\":\""
       <<(obs.error.empty()?"value":"error")<<"\",\"error\":";
    if(obs.error.empty())out<<"null";else out<<'"'<<obs.error<<'"';
    out<<",\"values\":"; if(obs.error.empty())array(out,obs.value);else out<<"null";out<<"}}";
}
} // namespace
int main(int argc,char** argv) {
    if(argc!=3)return 64;
    try {
        std::ifstream in(argv[1]); std::ofstream out(argv[2],std::ios::binary);
        if(!in||!out)return 2;
        in.imbue(std::locale::classic());out.imbue(std::locale::classic());
#ifdef _WIN32
        const auto pid=_getpid();
#else
        const auto pid=getpid();
#endif
        out<<"{\"schema_version\":2,\"kind\":\"cartesian-frames-certificate\",\"signed_zero_policy\":\"normalize_to_positive\",\"pid\":"<<pid<<",\"cases\":[";
        std::string line;bool first=true;
        while(std::getline(in,line)) {
            if(line.empty())continue;
            std::istringstream row(line);Case c;
            if(!(row>>c.id>>c.dimension>>c.operation>>c.kind>>c.direction>>c.category>>c.exponent))return 3;
            if(c.dimension!=2&&c.dimension!=3)return 3;
            for(auto* vec:{&c.origin,&c.basis,&c.operand,&c.auxiliary}) {
                const int count=vec==&c.basis?c.dimension*c.dimension:c.dimension;
                for(int i=0;i<count;++i)vec->push_back(number(row));
            }
            std::string extra;if(row>>extra)return 3;
            const auto observed=c.dimension==2?observe<2>(c):observe<3>(c);
            if(!first) out<<',';
            first=false;
            write_case(out,c,observed);
        }
        out<<"]}\n";return out.good()?0:4;
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 5;}
}
