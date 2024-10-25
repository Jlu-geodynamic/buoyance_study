/*
  Copyright (C) 2019 - 2020 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file LICENSE.  If not see
  <http://www.gnu.org/licenses/>.
*/

//新增错位蠕变
#include <aspect/material_model/rheology/dislocation_creep.h>
#include </fs2/home/liuzhonglan/wy/lib_extra/grainsize20230317/grain/include/rheology/diffusion_creep_grainsize.h>

#include <aspect/material_model/utilities.h>
#include <aspect/utilities.h>

#include <deal.II/base/signaling_nan.h>
#include <deal.II/base/parameter_handler.h>

#include <cmath>

// 此模型以material model中的visco plastic模型为基础修改而来，grain size相关计算参考aterial model中的grain size model模型
// 原文件位置以及名称：material_model/visco_plastic.cc 
//                     material_model/rheology/visco_plastic.cc 
//					   material_model/rheology/diffusion_creep.cc 
//					   material_model/grain_size.cc
namespace aspect
{
  namespace MaterialModel
  {
    namespace Rheology
    {
      template <int dim>
      DiffusionCreepGrainSize<dim>::DiffusionCreepGrainSize ()
      {}

      template <int dim>
      const DiffusionCreepGrainSizeParameters
      DiffusionCreepGrainSize<dim>::compute_creep_parameters (const unsigned int composition,
                                                     const std::vector<double> &phase_function_values,
                                                     const std::vector<unsigned int> &n_phases_per_composition) const
      {
        DiffusionCreepGrainSizeParameters creep_parameters;
        if (phase_function_values == std::vector<double>())
        {
            // no phases
            creep_parameters.prefactor_diff = prefactors_diffusion[composition];
            creep_parameters.activation_energy_diff = activation_energies_diffusion[composition];
            creep_parameters.activation_volume_diff = activation_volumes_diffusion[composition];
            creep_parameters.stress_exponent_diff = stress_exponents_diffusion[composition];
            creep_parameters.grain_size_exponent_diff = grain_size_exponents_diffusion[composition];
            creep_parameters.prefactor_dis = prefactors_dislocation[composition];
            creep_parameters.activation_energy_dis = activation_energies_dislocation[composition];
            creep_parameters.activation_volume_dis = activation_volumes_dislocation[composition];
            creep_parameters.stress_exponent_dis = stress_exponents_dislocation[composition];
            creep_parameters.grain_growth_exponent = grain_growth_exponent[composition];
            creep_parameters.grain_growth_rate_constant = grain_growth_rate_constant[composition];
            creep_parameters.grain_growth_activation_energy = grain_growth_activation_energy[composition];
            creep_parameters.grain_growth_activation_volume = grain_growth_activation_volume[composition];
            creep_parameters.boundary_area_change_work_fraction = boundary_area_change_work_fraction[composition];
            creep_parameters.geometric_constant = geometric_constant[composition];
            creep_parameters.grain_boundary_energy = grain_boundary_energy[composition];
        }
        else
          {
            // Average among phases
			//粒度计算模型中未启用此项
			creep_parameters.prefactor_diff = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													prefactors_diffusion, composition,  MaterialModel::MaterialUtilities::PhaseUtilities::logarithmic);
			creep_parameters.activation_energy_diff = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													activation_energies_diffusion, composition);
			creep_parameters.activation_volume_diff = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													activation_volumes_diffusion, composition);
			creep_parameters.stress_exponent_diff = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													stress_exponents_diffusion, composition);
			creep_parameters.grain_size_exponent_diff = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_size_exponents_diffusion, composition);
			creep_parameters.prefactor_dis = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													prefactors_dislocation, composition,  MaterialModel::MaterialUtilities::PhaseUtilities::logarithmic);
			creep_parameters.activation_energy_dis = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													activation_energies_dislocation, composition);
			creep_parameters.activation_volume_dis = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													activation_volumes_dislocation, composition);
			creep_parameters.stress_exponent_dis = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													stress_exponents_dislocation, composition);
			creep_parameters.grain_growth_exponent = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_growth_exponent, composition);
			creep_parameters.grain_growth_rate_constant = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_growth_rate_constant, composition,  MaterialModel::MaterialUtilities::PhaseUtilities::logarithmic);
			creep_parameters.grain_growth_activation_energy = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_growth_activation_energy, composition);
			creep_parameters.grain_growth_activation_volume = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_growth_activation_volume, composition);
			creep_parameters.boundary_area_change_work_fraction = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													boundary_area_change_work_fraction, composition);
			creep_parameters.geometric_constant = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													geometric_constant, composition);
			creep_parameters.grain_boundary_energy = MaterialModel::MaterialUtilities::phase_average_value(phase_function_values, n_phases_per_composition,
													grain_boundary_energy, composition);
          }
        return creep_parameters;
      }

	  //计算粒径变化，此处的计算参考了aspect中的grain size model
      template <int dim>
      double
      DiffusionCreepGrainSize<dim>::compute_grainsize(const double second_strain_rate_invariant,
													  const double pressure,
													  const double temperature,
													  const unsigned int composition,
													  const double last_grain_size,
													  const double posy,
													  const std::vector<double>& phase_function_values,
													  const std::vector<unsigned int>& n_phases_per_composition) const
      {
		  //1 拉伸状态下box整体处于抬升状态，导致最底部在计算时存在粒度异常的情况，因此在这里规定一定深度以下的粒径为模型设定的最大值
		  if (posy < banned_depth_meter)
			  return max_grain_size;
		  
		  //2 不开启粒度计算，或处于初始化阶段时
          if (flag[composition] == 0 || !this->simulator_is_past_initialization() || this->get_timestep_number() == 0) 
		  {
			  
			  //2.1 非岩石圈地幔区域返回初始值
			  if (composition != (this->introspection().compositional_index_for_name("mantle_L") + 1))
				  return initial_grain_size[composition];
			  
			  //2.2 岩石圈地幔初始粒度规则：从莫霍面至LAB面，令10指数的值按深度递增
		      //log10(grain_size) = init_mantle_grainsize + k * m 
		      //k是LAB面初始粒度指数值 - 莫霍面初始粒度指数值) / 岩石圈地幔厚度
		      //m是垂直方向到莫霍面的距离。
		      //设置粒度初始值时认定莫霍面为水平面。
		      else if (composition == (this->introspection().compositional_index_for_name("mantle_L") + 1))
		      {
		    	  const double lg_grainL_prm = log10(initial_grain_size[this->introspection().compositional_index_for_name("mantle_L") + 1]);
			      const double lg_grainA_prm = log10(initial_grain_size[0]);
			      const double lg_grain_k = (lg_grainA_prm - lg_grainL_prm) / thicknesses[2];
		    	  const double lg_grainL_init = lg_grainL_prm + lg_grain_k * (box_y - posy  - (thicknesses[0] + thicknesses[1]));
		    	  return pow(10, lg_grainL_init);
		      }
		  }

         const DiffusionCreepGrainSizeParameters p = compute_creep_parameters(composition,
													 phase_function_values,
													 n_phases_per_composition);
          //compute grain size
		  //本模型中的phase存放在compositional fields中，因此计算过程中不需要通过程序判断在哪个位置
	      //本模型并未选择进行循环计算，只根据步长进行一次计算，因此grain_growth_timestep为直接获取的步长
          const double grain_growth_timestep = this->get_timestep();
          double grain_size = last_grain_size;
          double current_dislocation_viscosity = 0.0;
          double current_diffusion_viscosity = 0.0;
          //grain size growth due to Ostwald ripening
          const double m = p.grain_growth_exponent;
          const double grain_size_growth_rate = p.grain_growth_rate_constant / (m * pow(grain_size, m - 1))
                                                * exp(-(p.grain_growth_activation_energy + pressure * p.grain_growth_activation_volume)
                                                / (constants::gas_constant * temperature));
          const double grain_size_growth = grain_size_growth_rate * grain_growth_timestep;
		  // grain size reduction in dislocation creep regime
		  current_diffusion_viscosity = 0.5 / p.prefactor_diff *
										std::exp((p.activation_energy_diff +
										pressure * p.activation_volume_diff) /
										(constants::gas_constant * temperature)) *
										std::pow(grain_size, p.grain_size_exponent_diff);
		  current_diffusion_viscosity = std::min(current_diffusion_viscosity, std::sqrt(std::numeric_limits<double>::max()));
		  //dislocation creep并未选取grain size model中的迭代计算方式
		  current_dislocation_viscosity = 0.5 * std::pow(p.prefactor_dis, -1 / p.stress_exponent_dis) *
										  std::exp((p.activation_energy_dis + pressure * p.activation_volume_dis) /
										  (constants::gas_constant * temperature * p.stress_exponent_dis)) *
										  std::pow(second_strain_rate_invariant, ((1. - p.stress_exponent_dis) / p.stress_exponent_dis));
		  current_dislocation_viscosity = std::min(current_dislocation_viscosity, std::sqrt(std::numeric_limits<double>::max()));
		  //本模型的有效粘度在prm文件中定义。
		  const double current_viscosity = current_dislocation_viscosity * current_diffusion_viscosity / (current_dislocation_viscosity + current_diffusion_viscosity);
		  //计算粒度减小时，认为xxx的区域不发生变形
		  const double dis_edot_ii = second_strain_rate_invariant > edot_test ? second_strain_rate_invariant : 1e-30;
		  const double dislocation_strain_rate = dis_edot_ii * current_viscosity / current_dislocation_viscosity;
		  // paleowattmeter: Austin and Evans (2007): Paleowattmeters: A scaling relation for dynamically recrystallized grain size. Geology 35, 343-346
		  const double stress = 2.0 * dis_edot_ii * current_viscosity;
		  const double grain_size_reduction_rate = 2.0 * stress * p.boundary_area_change_work_fraction * dislocation_strain_rate * pow(grain_size, 2)
													/ (p.geometric_constant * p.grain_boundary_energy);
		  const double grain_size_reduction = grain_size_reduction_rate * grain_growth_timestep;
		  const double grain_size_change = grain_size_growth - grain_size_reduction;
		  grain_size += grain_size_change;
		  Assert(grain_size > 0,
			   ExcMessage("The grain size became smaller than zero. This is not valid, "
						  "and likely an effect of a too large sub-timestep, or unrealistic "
						  "input parameters."));
		  return std::max(std::min(grain_size, max_grain_size) ,  minimum_grain_size);
      }

      template <int dim>
      double
      DiffusionCreepGrainSize<dim>::compute_viscosity (const double pressure,
													   const double temperature,
												  	   const unsigned int composition,
												 	   const double grain_size,
													   const std::vector<double> &phase_function_values,
													   const std::vector<unsigned int> &n_phases_per_composition) const
      {
              
        const DiffusionCreepGrainSizeParameters p = compute_creep_parameters(composition,
                                                                    phase_function_values,
                                                                    n_phases_per_composition);
        // Power law creep equation
        //    viscosity = 0.5 * A^(-1/n) * d^(m/n) * exp((E + P*V)/(nRT))
        // A: prefactor,
        // d: grain size, m: grain size exponent, E: activation energy, P: pressure,
        // V; activation volume, R: gas constant, T: temperature.
        double viscosity_diffusion = 0.5 / p.prefactor_diff *
                                     std::exp((p.activation_energy_diff +
                                               pressure*p.activation_volume_diff)/
                                              (constants::gas_constant*temperature)) *
                                     std::pow(grain_size, p.grain_size_exponent_diff);
        Assert (viscosity_diffusion > 0.0,
                ExcMessage ("Negative diffusion viscosity detected. This is unphysical and should not happen. "
                            "Check for negative parameters."));
        // Creep viscosities become extremely large at low
        // temperatures and can therefore provoke floating-point overflow errors. In
        // real rocks, other deformation mechanisms become dominant at low temperatures,
        // so these high viscosities are never achieved. It is therefore both reasonable
        // and desirable to require the single-mechanism viscosity to be smaller than
        // std::sqrt(max_double).
        viscosity_diffusion = std::min(viscosity_diffusion, std::sqrt(std::numeric_limits<double>::max()));
        return viscosity_diffusion;
      }



      template <int dim>
      std::pair<double, double>
      DiffusionCreepGrainSize<dim>::compute_strain_rate_and_derivative (const double stress,
                                                               const double pressure,
                                                               const double temperature,
                                                               const double grain_size,
                                                               const DiffusionCreepGrainSizeParameters creep_parameters) const
      {
        // Power law creep equation
        //   edot_ii_partial = A * stress^n * d^-m * exp(-(E + P*V)/(RT))
        //   d(edot_ii_partial)/d(stress) = A * n * stress^(n-1) * d^-m * exp(-(E + P*V)/(RT))
        // A: prefactor, edot_ii_partial: square root of second invariant of deviatoric strain rate tensor attributable to the creep mechanism,
        // d: grain size, m: grain size exponent, E: activation energy, P: pressure,
        // V; activation volume, R: gas constant, T: temperature.
        // For diffusion creep, n = 1 (strain rate is linearly dependent on stress).
        const double dstrain_rate_dstress_diffusion = creep_parameters.prefactor_diff *
                                                      std::pow(grain_size, -creep_parameters.grain_size_exponent_diff) *
                                                      std::exp(-(creep_parameters.activation_energy_diff + pressure*creep_parameters.activation_volume_diff)/
                                                               (constants::gas_constant*temperature));

        const double strain_rate_diffusion = stress * dstrain_rate_dstress_diffusion;

        return std::make_pair(strain_rate_diffusion, dstrain_rate_dstress_diffusion);
      }



      template <int dim>
      void
      DiffusionCreepGrainSize<dim>::declare_parameters (ParameterHandler &prm)
      {
        prm.declare_entry ("Prefactors for diffusion creep", "1.5e-15",
                           Patterns::Anything(),
                           "List of viscosity prefactors, $A$, for background material and compositional fields, "
                           "for a total of N+1 values, where N is the number of compositional fields. "
                           "If only one value is given, then all use the same value. "
                           "Units: \\si{\\per\\pascal\\meter}$^{m_{\\text{diffusion}}}$\\si{\\per\\second}.");
        prm.declare_entry ("Stress exponents for diffusion creep", "1.",
                           Patterns::List(Patterns::Double(0.)),
                           "List of stress exponents, $n_{\\text{diffusion}}$, for background mantle and compositional fields, "
                           "for a total of N+1 values, where N is the number of compositional fields. "
                           "The stress exponent for diffusion creep is almost always equal to one. "
                           "If only one value is given, then all use the same value.  Units: None.");
        prm.declare_entry ("Grain size exponents for diffusion creep", "3.",
                           Patterns::Anything(),
                           "List of grain size exponents, $m_{\\text{diffusion}}$, for background material and compositional fields, "
                           "for a total of N+1 values, where N is the number of compositional fields. "
                           "If only one value is given, then all use the same value. Units: None.");
        prm.declare_entry ("Activation energies for diffusion creep", "375e3",
                           Patterns::Anything(),
                           "List of activation energies, $E_a$, for background material and compositional fields, "
                           "for a total of N+1 values, where N is the number of compositional fields. "
                           "If only one value is given, then all use the same value. "
                           "Units: \\si{\\joule\\per\\mole}.");
        prm.declare_entry ("Activation volumes for diffusion creep", "6e-6",
                           Patterns::Anything(),
                           "List of activation volumes, $V_a$, for background material and compositional fields, "
                           "for a total of N+1 values, where N is the number of compositional fields. "
                           "If only one value is given, then all use the same value. "
                           "Units: \\si{\\meter\\cubed\\per\\mole}.");
		prm.declare_entry ("Minimum Grain Size", "5e-6",
						 Patterns::Double (0.),
						 "The minimum allowable grain size. The grain size will be limited to be "
						 "larger than this value. This can be used to damp out oscillations, or "
						 "to limit the viscosity variation due to grain size. "
						 "Units: \\si{\\meter}.");
		prm.declare_entry("Initial Time Step", "500",
						 Patterns::Double(0.),
						 "Units: none");
		prm.declare_entry ("Advect logarithm of grain size", "0",
						 Patterns::List (Patterns::Double (0.)),
						 "This parameter determines whether to advect the logarithm of the grain size "
						 "or the grain size itself. The equation and the physics are the same, "
						 "but for problems with high grain size gradients it might "
						 "be preferable to advect the logarithm. ");
		prm.declare_entry ("Grain size calculation flag", "0",
						 Patterns::List (Patterns::Double (0.)),
						 "A flag of determining whether calculating grain size. "
						 "It will use a static value of grain size when the flag sets false. ");
		prm.declare_entry ("Grain size minus flag", "0",
						 Patterns::List (Patterns::Double (0.)),
						 "Testing flag. "
						 "When it set true, result of grain size will return: grain size - old grain size. ");
		prm.declare_entry ("Grain growth exponent", "3.",
						 Patterns::List (Patterns::Double (0.)),
						 "The exponent of the grain growth law $p_g$. This is an experimentally determined "
						 "grain growth constant. "
						 "Units: none.");
		prm.declare_entry ("Grain growth rate constant", "1.5e-5",
						 Patterns::List (Patterns::Double (0.)),
						 "The prefactor for the Ostwald ripening grain growth law $G_0$. "
						 "This is dependent on water content, which is assumed to be "
						 "50 H/$10^6$ Si for the default value. "
						 "Units: \\si{\\meter}$^{p_g}$\\si{\\per\\second}.");
		prm.declare_entry ("Grain growth activation energy", "3.5e5",
						 Patterns::List (Patterns::Double (0.)),
						 "The activation energy for grain growth $E_g$. "
						 "Units: \\si{\\joule\\per\\mole}.");
		prm.declare_entry ("Grain growth activation volume", "8e-6",
						 Patterns::List (Patterns::Double (0.)),
						 "The activation volume for grain growth $V_g$. "
						 "Units: \\si{\\meter\\cubed\\per\\mole}.");
		prm.declare_entry ("Work fraction for boundary area change", "0.1",
						 Patterns::List (Patterns::Double (0.)),
						 "The fraction $\\chi$ of work done by dislocation creep to change the grain boundary area. "
						 "Units: \\si{\\joule\\per\\meter\\squared}.");
		prm.declare_entry ("Geometric constant", "3.",
						 Patterns::List (Patterns::Double (0.)),
						 "The geometric constant $c$ used in the paleowattmeter grain size reduction law. "
						 "Units: none.");
		prm.declare_entry ("Average specific grain boundary energy", "1.0",
						 Patterns::List (Patterns::Double (0.)),
						 "The average specific grain boundary energy $\\gamma$. "
						 "Units: \\si{\\joule\\per\\meter\\squared}.");
		prm.declare_entry("Initial Grain size", "1e-3",
						  Patterns::Anything(),
						  "Units: \\si{\\meter}.");
		prm.declare_entry("Depth of banning grain calculation", "2000",
						  Patterns::Double(0.),
						 "Units: meter.");
		prm.declare_entry("Maximum Grain Size", "5e-2",
						  Patterns::Double(0.),
						 "Units: meter.");
		prm.declare_entry ("GrainInit Layer thicknesses", "30000.",
                             Patterns::List(Patterns::Double(0)),
                             "List of thicknesses for the bottom of the lithospheric layers,"
                             "for a total of N+1 values, where N is the number of compositional fields."
                             "If only one value is given, then all use the same value.  Units: $m$");
        prm.declare_entry("GrainInit Box Y extent", "30000.",
                            Patterns::List(Patterns::Double(0)),
                            "Units: $m$");
	    prm.declare_entry("edotii limit for grain size reduction", "5e-16",
                            Patterns::List(Patterns::Double(0)),
                            "Units: $m$");
      }

      template <int dim>
      void
      DiffusionCreepGrainSize<dim>::parse_parameters (ParameterHandler &prm,
                                             const std::shared_ptr<std::vector<unsigned int>> &expected_n_phases_per_composition)
      {
		  
        // Retrieve the list of composition names
        const std::vector<std::string> list_of_composition_names = this->introspection().get_composition_names();
        // Establish that a background field is required here
        const bool has_background_field = true;
		
		// Read parameters, each of size of number of composition + number of phases + 1
		minimum_grain_size = prm.get_double("Minimum Grain Size");
		max_grain_size = prm.get_double("Maximum Grain Size");
		banned_depth_meter = prm.get_double("Depth of banning grain calculation");
		prefactors_diffusion = Utilities::parse_map_to_double_array(prm.get("Prefactors for diffusion creep"),
																	list_of_composition_names,
																	has_background_field,
																	"Prefactors for diffusion creep",
																	true,
																	expected_n_phases_per_composition);
		stress_exponents_diffusion = Utilities::parse_map_to_double_array(prm.get("Stress exponents for diffusion creep"),
																		  list_of_composition_names,
																		  has_background_field,
																		  "Stress exponents for diffusion creep",
																		  true,
																		  expected_n_phases_per_composition);
		grain_size_exponents_diffusion = Utilities::parse_map_to_double_array(prm.get("Grain size exponents for diffusion creep"),
																			  list_of_composition_names,
																			  has_background_field,
																			  "Grain size exponents for diffusion creep",
																			  true,
																			  expected_n_phases_per_composition);
		activation_energies_diffusion = Utilities::parse_map_to_double_array(prm.get("Activation energies for diffusion creep"),
																			 list_of_composition_names,
																			 has_background_field,
																			 "Activation energies for diffusion creep",
																			 true,
																			 expected_n_phases_per_composition);
		activation_volumes_diffusion = Utilities::parse_map_to_double_array(prm.get("Activation volumes for diffusion creep"),
																			list_of_composition_names,
																			has_background_field,
																			"Activation volumes for diffusion creep",
																			true,
																			expected_n_phases_per_composition);
		flag = Utilities::parse_map_to_double_array(prm.get("Grain size calculation flag"),
																list_of_composition_names,
																has_background_field,
																"Grain size calculation flag",
																true,
																expected_n_phases_per_composition);
		grain_growth_exponent = Utilities::parse_map_to_double_array(prm.get("Grain growth exponent"),
																		list_of_composition_names,
																		has_background_field,
																		"Grain growth exponent",
																		true,
																		expected_n_phases_per_composition);
		grain_growth_rate_constant = Utilities::parse_map_to_double_array(prm.get("Grain growth rate constant"),
																			list_of_composition_names,
																			has_background_field,
																			"Grain growth rate constant",
																			true,
																			expected_n_phases_per_composition);
		grain_growth_activation_energy = Utilities::parse_map_to_double_array(prm.get("Grain growth activation energy"),
																				list_of_composition_names,
																				has_background_field,
																				"Grain growth activation energy",
																				true,
																				expected_n_phases_per_composition);
		grain_growth_activation_volume = Utilities::parse_map_to_double_array(prm.get("Grain growth activation volume"),
																				list_of_composition_names,
																				has_background_field,
																				"Grain growth activation volume",
																				true,
																				expected_n_phases_per_composition);
		boundary_area_change_work_fraction = Utilities::parse_map_to_double_array(prm.get("Work fraction for boundary area change"),
																					list_of_composition_names,
																					has_background_field,
																					"Work fraction for boundary area change",
																					true,
																					expected_n_phases_per_composition);
		geometric_constant = Utilities::parse_map_to_double_array(prm.get("Geometric constant"),
																	list_of_composition_names,
																	has_background_field,
																	"Geometric constant",
																	true,
																	expected_n_phases_per_composition);
		grain_boundary_energy = Utilities::parse_map_to_double_array(prm.get("Average specific grain boundary energy"),
																		list_of_composition_names,
																		has_background_field,
																		"Average specific grain boundary energy",
																		true,
																		expected_n_phases_per_composition);																  
		prefactors_dislocation = Utilities::parse_map_to_double_array(prm.get("Prefactors for dislocation creep"),
																		  list_of_composition_names,
																		  has_background_field,
																		  "Prefactors for dislocation creep",
																		  true,
																		  expected_n_phases_per_composition);
		stress_exponents_dislocation = Utilities::parse_map_to_double_array(prm.get("Stress exponents for dislocation creep"),
																				list_of_composition_names,
																				has_background_field,
																				"Stress exponents for dislocation creep",
																				true,
																				expected_n_phases_per_composition);
		activation_energies_dislocation = Utilities::parse_map_to_double_array(prm.get("Activation energies for dislocation creep"),
																				   list_of_composition_names,
																				   has_background_field,
																				   "Activation energies for dislocation creep",
																				   true,
																				   expected_n_phases_per_composition);
		activation_volumes_dislocation  = Utilities::parse_map_to_double_array(prm.get("Activation volumes for dislocation creep"),
																				   list_of_composition_names,
																				   has_background_field,
																				   "Activation volumes for dislocation creep",
																				   true,
																				   expected_n_phases_per_composition);
		initial_grain_size = Utilities::parse_map_to_double_array(prm.get("Initial Grain size"),
																	list_of_composition_names,
																	has_background_field,
																	"Initial Grain size",
																	true,
																	expected_n_phases_per_composition);
		thicknesses = Utilities::possibly_extend_from_1_to_N (Utilities::string_to_double(Utilities::split_string_list(prm.get("GrainInit Layer thicknesses"))),
																3,
																"GrainInit Layer thicknesses");
		box_y = prm.get_double("GrainInit Box Y extent");
		edot_test = prm.get_double("edotii limit for grain size reduction");
      }
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace MaterialModel
  {
#define INSTANTIATE(dim) \
  namespace Rheology \
  { \
    template class DiffusionCreepGrainSize<dim>; \
  }

    ASPECT_INSTANTIATE(INSTANTIATE)

#undef INSTANTIATE
  }
}
