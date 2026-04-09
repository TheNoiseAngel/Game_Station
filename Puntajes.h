#ifndef PUNTAJES_H
#define PUNTAJES_H

#include <fstream>
#include <string>
#include <map>

namespace SysPuntaje
{
std::map<std::string, int> records;
void cargar()
{
	std::ifstream Arch("Records.pointsG");
	std::string J;
	int pts;
	while (Arch >> J >> pts)
	{
		records[J] = pts;
	}
	Arch.close();
}

void guardar()
{
	std::ofstream Arch("Records.pointsG");
	for (auto const &[J, pts] : records)
	{
		Arch << J << " " << pts << "\n";
	}
	Arch.close();
}

void actualizar(std::string const &J, const int &n_pts)
{
	if (n_pts > records[J])
	{
		records[J] = n_pts;
	}
	guardar();
}
} // namespace SysPuntaje
#endif

//int main()
//{

//	return 0;
//}